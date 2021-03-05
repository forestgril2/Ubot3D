#include "GCodeProgramParser.h"

#include <gcode_program.h>
#include <parser.h>

using ExtrPath = Extrusion::Path;
using ExtrPoint = Extrusion::Point;
using ExtrLayer = Extrusion::Layer;

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
	assert(extruderPaths->size() != 0);
	const ExtrPath pathPrev = (*extruderPaths)[extruderPaths->size() -2];

	ExtrPoint previousLayerLastPoint;
	if (pathPrev.size() > 0)
	{
		previousLayerLastPoint = pathPrev[pathPrev.size() -1];
	}
	else
	{
		previousLayerLastPoint = {0,0,0,0};
	}

	extruderCurr->layers.push_back({extruderPaths->size(), previousLayerLastPoint.z()});
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
	extruderCurr = &extruders[0];
	extruderPaths = &(extruderCurr->paths);

	std::vector<ExtrLayer>& layerBottoms = extruderCurr->layers;
	// Layer bottom at start is just the bed level.
	layerBottoms.push_back({0u, 0.0f});

	unsigned blockCount = 0;
	const unsigned blockCountLimit = 5220800;

	ExtrPoint lastAbsCoords(0,0,0,0);

	for (auto it = gcodeProgram.begin();
		 it != gcodeProgram.end() && blockCount < blockCountLimit;
		 ++it, ++blockCount)
	{
		const auto& block = *it;
		blockStringCurr = block.to_string();
		ExtrPoint blockCurrRelativeCoords(0,0,0,0);

		Vector4i whichCoordsSetInBlock(0,0,0,0);

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
					const gpr::addr& adddres = chunk.get_address();
					const char& word = chunk.get_word();

					switch(word)
					{
						// Extruder choice.
						case 'T':
							switch (adddres.tp())
							{
								case gpr::ADDRESS_TYPE_INTEGER:
									setExtruder(adddres.int_value());
									break;

								case gpr::ADDRESS_TYPE_DOUBLE:
									assert(false);
									break;
							}
							break;

						// Header movements.
						case 'G':
							switch (adddres.tp())
							{
								case gpr::ADDRESS_TYPE_INTEGER:
									switch (adddres.int_value())
									{
										case 0:
											setExtrusionOff(extruderCurr);
											break;
										case 1:
											setExtrusionOn(extruderCurr, lastAbsCoords);
											break;
										case 90:
											setAbsoluteModeOn(blockCurrAbsCoords);
											break;
										case 91:
											setAbsoluteModeOff(blockCurrRelativeCoords);
											break;
										case 92:
											//TODO: IMPLEMENT.
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
							newCoordsCurr->x() = float(adddres.double_value());
							whichCoordsSetInBlock.x() = true;
							break;
						case 'Y':
							newCoordsCurr->y() = float(adddres.double_value());
							whichCoordsSetInBlock.y() = true;
							break;
						case 'Z':
							newCoordsCurr->z() = float(adddres.double_value());
							whichCoordsSetInBlock.z() = true;
							break;

						// Extruded length.
						case 'E':
							newCoordsCurr->w() = float(adddres.double_value());
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
			blockCurrAbsCoords[i] = lastAbsCoords[i];
		}

		if (!isAbsoluteMode)
		{
			lastAbsCoords += blockCurrRelativeCoords;
		}
		else
		{
			lastAbsCoords = blockCurrAbsCoords;
		}

		if (extruderCurr->isExtruding && (pathCurr.size() == 0 || lastAbsCoords != pathCurr.back()))
		{// Ignore movements without extrusion and identical extruder path coordinates.
			pathCurr.push_back(lastAbsCoords);
		}
	}

	if (!pathCurr.empty())
	{
		numPathPointsMax = std::max<size_t>(numPathPointsMax, pathCurr.size());
		std::cout << " #### adding subPath no. " << extruderPaths->size() -1<< ", maxPointsInSubPath: " << numPathPointsMax << std::endl;

//		dumpSubPath(blockStringCurr, subPathCurr);

		std::swap(extruderPaths->back(), pathCurr);
	}

	extruderCurr->numPathPointsMax = numPathPointsMax;
	extruderCurr->numPaths = (*extruderPaths).size();

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

	if (pathCurr.empty())
		return;
	numPathPointsMax = std::max<unsigned>(numPathPointsMax, pathCurr.size());

//	dumpSubPath(blockStringCurr, subPathCurr);

	if (pathCurr.size() < 2)
	{
		static bool wasSubpathOfSmallSizeAnounced = false;
		if (!wasSubpathOfSmallSizeAnounced)
		{
			std::cout << "### WARNING: subpath of size: " << pathCurr.size() << std::endl;
			wasSubpathOfSmallSizeAnounced = true;
		}

		pathCurr.clear();
		return;
	}

	std::swap(extruderPaths->back(), pathCurr);
	extruderPaths->push_back(ExtrPath());
}

void GCodeProgramParser::setExtrusionOn(Extrusion* extruder, const ExtrPoint& lastAbsCoords)
{// If we are setting extrusion on, add new (empty) path to path vector.
	if (extruder->isExtruding)
		return;
	extruder->isExtruding = true;
	pathCurr.push_back(lastAbsCoords);
}

void GCodeProgramParser::setAbsoluteModeOn(ExtrPoint& blockCurrAbsCoords)
{
	if (isAbsoluteMode)
		return;
	isAbsoluteMode = true;
	newCoordsCurr = &blockCurrAbsCoords;
}

void GCodeProgramParser::setAbsoluteModeOff(Extrusion::Point& blockCurrRelativeCoords)
{
	if (!isAbsoluteMode)
		return;

	isAbsoluteMode = false;
	newCoordsCurr = &blockCurrRelativeCoords;
}
