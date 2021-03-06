#include "GCodeProgramParser.h"

#include <gcode_program.h>
#include <parser.h>

static gpr::gcode_program importGCodeFromFile(const std::string& file)
{
	Chronograph chronograph("Read gcode file contents", true);
	std::ifstream t(file);
	std::string file_contents((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	chronograph.log();

	chronograph.start("Parse gcode");
	return gpr::parse_gcode(file_contents);
}

void GCodeProgramParser::pushNewLayer()
{
	assert(_extruderPathsCurr->size() != 0);
	const ExtrPath pathPrev = (*_extruderPathsCurr)[_extruderPathsCurr->size() -2];

	ExtrPoint previousLayerLastPoint;
	if (pathPrev.size() > 0)
	{
		previousLayerLastPoint = pathPrev[pathPrev.size() -1];
	}
	else
	{
		previousLayerLastPoint = {0,0,0,0};
	}

	_extruderCurr->layers.push_back({_extruderPathsCurr->size(), previousLayerLastPoint.z()});
}

Extrusion GCodeProgramParser::initializeExtruderData()
{
	Extrusion newData;
	newData.paths.clear();
	newData.paths.push_back(ExtrPath());
	return newData;
}

void GCodeProgramParser::setExtruder(const uint32_t extruderNumber)
{

}

bool GCodeProgramParser::isNewLayerComment(const std::string& comment)
{
	// First layer has bed level (0.0f), like all preparation steps. We return true only for layers 1+
	return
			// Cura format ;FLAVOR:Marlin
			(comment._Starts_with("LAYER:") && (0 != comment.compare("LAYER:0")))
			||// Ubot3D format
			(comment._Starts_with("beforelayer") && (0 != comment.compare("firstlayer")));
}

std::vector<Extrusion> GCodeProgramParser::createExtrusionData(const std::string& inputFile)
{
	Chronograph chronograph(__FUNCTION__, true);

	gpr::gcode_program gcodeProgram = importGCodeFromFile(inputFile);

	// Start with a one extruder, add new if we encounter T1+ block.
	std::vector<Extrusion> extruders{initializeExtruderData()};
	_extruderCurr = &extruders[0];
	_extruderPathsCurr = &(_extruderCurr->paths);
	_blockCurrAbsCoordsCurr = {0,0,0,0};
	_newCoordsCurr = &_blockCurrAbsCoordsCurr;

	std::vector<ExtrLayer>& layerBottoms = _extruderCurr->layers;
	// Layer bottom at start is just the bed level.
	layerBottoms.push_back({0u, 0.0f});

	unsigned blockCount = 0;
	const unsigned blockCountLimit = 5220800;

	ExtrPoint lastAbsCoords(0,0,0,0);

	for (std::vector<gpr::block>::iterator it = gcodeProgram.begin();
		 it != gcodeProgram.end() && blockCount < blockCountLimit;
		 ++it, ++blockCount)
	{
		const gpr::block& block = *it;
		ExtrPoint blockCurrRelativeCoords(0,0,0,0);
		Vector4i whichCoordsSetInBlock(0,0,0,0);

		_blockStringCurr = block.to_string();
//		std::cout << " ### " << __FUNCTION__ << " _blockStringCurr:" << _blockStringCurr << "," << "" << std::endl;

		for (const gpr::chunk& chunk : block)
		{
			switch(chunk.tp())
			{
				case gpr::CHUNK_TYPE_PERCENT:
				case gpr::CHUNK_TYPE_WORD:
					break;

				case gpr::CHUNK_TYPE_COMMENT:
				{
					if (!isNewLayerComment(chunk.get_comment_text()))
						continue;

					pushNewLayer();
					break;
				}
				case gpr::CHUNK_TYPE_WORD_ADDRESS:
				{
					const gpr::addr& addres = chunk.get_address();
					const char& word = chunk.get_word();

					switch(word)
					{
						// Extruder choice.
						case 'T':
							switch (addres.tp())
							{
								case gpr::ADDRESS_TYPE_INTEGER:
//									setExtruder(adddres.int_value());
									break;

								case gpr::ADDRESS_TYPE_DOUBLE:
									assert(false);
									break;
							}
							break;

						// Header movements.
						case 'G':
							switch (addres.tp())
							{
								case gpr::ADDRESS_TYPE_INTEGER:
									switch (addres.int_value())
									{
										case 0:
											setExtrusionOff(_extruderCurr);
											break;
										case 1:
											setExtrusionOn(_extruderCurr, lastAbsCoords);
											break;
										case 90:
											setAbsoluteModeOn();
											break;
										case 91:
											setAbsoluteModeOff(&blockCurrRelativeCoords);
											break;
										case 92:
//											setPosition()
											break;
										default:
											break;
									}
									break;

								case gpr::ADDRESS_TYPE_DOUBLE:
									assert(false);
									break;
							}
							break;

						// Path point coordinates.
						case 'X':
							_newCoordsCurr->x() = float(addres.double_value());
							whichCoordsSetInBlock.x() = true;
							break;
						case 'Y':
							_newCoordsCurr->y() = float(addres.double_value());
							whichCoordsSetInBlock.y() = true;
							break;
						case 'Z':
							_newCoordsCurr->z() = float(addres.double_value());
							whichCoordsSetInBlock.z() = true;
							break;

						// Extruded length.
						case 'E':
							_newCoordsCurr->w() = float(addres.double_value());
							whichCoordsSetInBlock.w() = true;
							break;

						default:
							break;
					}
					break;
				}
			}
		}

		if (whichCoordsSetInBlock.isZero())
			continue;

		for (unsigned short i = 0; i < whichCoordsSetInBlock.size(); ++i)
		{// updateBlockCoords(): If this coord is not set in this block, use the most recent for absolute coordinates.
			if (whichCoordsSetInBlock[i] != 0 || !isAbsoluteMode)
				continue;
			_blockCurrAbsCoordsCurr[i] = lastAbsCoords[i];
		}

		if (!isAbsoluteMode)
		{
			lastAbsCoords += blockCurrRelativeCoords;
		}
		else
		{
			lastAbsCoords = _blockCurrAbsCoordsCurr;
		}

		if (_extruderCurr->isExtruding && (_pathCurr.size() == 0 || lastAbsCoords != _pathCurr.back()))
		{// Ignore movements without extrusion and identical extruder path coordinates.
			_pathCurr.push_back(lastAbsCoords);
		}
	}

	if (!_pathCurr.empty())
	{
		numPathPointsMax = std::max<size_t>(numPathPointsMax, _pathCurr.size());
		std::cout << " #### adding subPath no. " << _extruderPathsCurr->size() -1<< ", maxPointsInSubPath: " << numPathPointsMax << std::endl;

//		dumpSubPath(blockStringCurr, subPathCurr);

		std::swap(_extruderPathsCurr->back(), _pathCurr);
	}

	_extruderCurr->numPathPointsMax = numPathPointsMax;
	_extruderCurr->numPaths = (*_extruderPathsCurr).size();

	return extruders;
}

void GCodeProgramParser::dumpSubPath(const std::string& blockString, const Extrusion::Path& path)
{
	std::ofstream pathFile("subPathDump.txt");
	for (const ExtrPoint& point : path)
	{
		pathFile << "[" << point.x() << "," << point.y() << "," << point.z() << "," << point.w() << "]" << std::endl;
	}
	pathFile << blockString << std::endl;
	pathFile.close();
}

void GCodeProgramParser::setExtrusionOff(Extrusion* extruder)
{// If we are setting extrusion off, swap created path with the empty one in path vector.
	if (!extruder->isExtruding)
		return;
	extruder->isExtruding = false;

	if (_pathCurr.empty())
		return;
	numPathPointsMax = std::max<unsigned>(numPathPointsMax, _pathCurr.size());

//	dumpSubPath(blockStringCurr, subPathCurr);

	if (_pathCurr.size() < 2)
	{
		static bool wasSubpathOfSmallSizeAnounced = false;
		if (!wasSubpathOfSmallSizeAnounced)
		{
			std::cout << "### WARNING: subpath of size: " << _pathCurr.size() << std::endl;
			wasSubpathOfSmallSizeAnounced = true;
		}

		_pathCurr.clear();
		return;
	}

	std::swap(_extruderPathsCurr->back(), _pathCurr);
	_extruderPathsCurr->push_back(ExtrPath());
}

void GCodeProgramParser::setExtrusionOn(Extrusion* extruder, const ExtrPoint& lastAbsCoords)
{// If we are setting extrusion on, add new (empty) path to path vector.
	if (extruder->isExtruding)
		return;
	extruder->isExtruding = true;
	_pathCurr.push_back(lastAbsCoords);
}

void GCodeProgramParser::setAbsoluteModeOn()
{
	if (isAbsoluteMode)
		return;
	isAbsoluteMode = true;
	_newCoordsCurr = &_blockCurrAbsCoordsCurr;
}

void GCodeProgramParser::setAbsoluteModeOff(ExtrPoint* blockCurrRelativeCoords)
{
	if (!isAbsoluteMode)
		return;

	isAbsoluteMode = false;
	_newCoordsCurr = blockCurrRelativeCoords;
}
