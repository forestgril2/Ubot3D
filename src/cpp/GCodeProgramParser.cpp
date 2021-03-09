#include "GCodeProgramParser.h"

#include <gcode_program.h>
#include <parser.h>

// That would be around 1GB GCode probably.
static const uint32_t blockCountLimit = 64'000'000;
static uint32_t blockCount = 0;


static gpr::gcode_program importGCodeFromFile(const std::string& file)
{
	Chronograph chronograph("Read gcode file contents", true);
	std::ifstream t(file);
	std::string file_contents((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	chronograph.log();

	blockCount = 0;
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
	// Layer bottom at start is just the bed level.
	newData.layers.push_back({0u, 0.0f});
	return newData;
}

void GCodeProgramParser::setExtruder(const uint32_t extruderNumber)
{

}

bool GCodeProgramParser::isNewLayerComment(const std::string& comment)
{// First layer has bed level (0.0f), like all preparation steps. We return true only for layers 1+
	return
			// Cura format ;FLAVOR:Marlin
			(comment._Starts_with("LAYER:") && (0 != comment.compare("LAYER:0")))
			||// Ubot3D format
			(comment._Starts_with("beforelayer") && (0 != comment.compare("firstlayer")));
}

bool GCodeProgramParser::isNewPathPoint() const
{// Ignore movements without extrusion and identical extruder path coordinates.
	return _extruderCurr->isExtruding &&
			(_pathCurr.size() == 0 || extrWorkPoints.lastAbsCoords != _pathCurr.back());
}

void GCodeProgramParser::initializeCurrentExtruderReferences(std::vector<Extrusion>& extruders)
{
	_extruderCurr = &extruders[0];
	_extruderPathsCurr = &(_extruderCurr->paths);
	_blockCurrAbsCoordsCurr = {0,0,0,0};
	_newCoordsCurr = &_blockCurrAbsCoordsCurr;
}

bool GCodeProgramParser::parseComment(const std::string& comment)
{
	if (!isNewLayerComment(comment))
		return false;

	pushNewLayer();
	return true;
}

void GCodeProgramParser::switchExtruderModes(const int value)
{
	switch (value)
	{
		case 0:
			setExtrusionOff(_extruderCurr);
			return;
		case 1:
			setExtrusionOn(_extruderCurr, extrWorkPoints.lastAbsCoords);
			return;
		case 90:
			setAbsoluteModeOn();
			return;
		case 91:
			setAbsoluteModeOff(&extrWorkPoints.blockCurrRelativeCoords);
			return;
		case 92:
			//setPosition()
			return;
	}
}

void GCodeProgramParser::parseWordAddressChunk(const char word, const gpr::addr& address)
{
	switch(word)
	{
		// Extruder choice.
		case 'T':
			switch (address.tp())
			{
				case gpr::ADDRESS_TYPE_INTEGER:
					//setExtruder(adddres.int_value());
					break;
				case gpr::ADDRESS_TYPE_DOUBLE:
					assert(false);
					break;
			}
			break;

		// Header movements.
		case 'G':
			switch (address.tp())
			{
				case gpr::ADDRESS_TYPE_INTEGER:
					switchExtruderModes(address.int_value());
					break;

				case gpr::ADDRESS_TYPE_DOUBLE:
					assert(false);
					break;
			}
			break;

		// Path point coordinates.
		case 'X':
		case 'Y':
		case 'Z':
		case 'E':
			handleExtrCoordSetting(word, Real(address.double_value()));
		default:
			break;
	}
}

void GCodeProgramParser::handleExtrCoordSetting(const char coord, const Real value)
{
	switch(coord)
	{
		case 'X':
			_newCoordsCurr->x() = value;
			extrWorkPoints.whichCoordsSetInBlock.x() = true;
			break;
		case 'Y':
			_newCoordsCurr->y() = value;
			extrWorkPoints.whichCoordsSetInBlock.y() = true;
			break;
		case 'Z':
			_newCoordsCurr->z() = value;
			extrWorkPoints.whichCoordsSetInBlock.z() = true;
			break;
		case 'E':
			// Extruded length.
			_newCoordsCurr->w() = value;
			extrWorkPoints.whichCoordsSetInBlock.w() = true;
			break;
		default:
			assert(false);
	}
}

void GCodeProgramParser::parseChunks(const gpr::block& block)
{
	for (const gpr::chunk& chunk : block)
	{
		switch(chunk.tp())
		{
			case gpr::CHUNK_TYPE_PERCENT:
			case gpr::CHUNK_TYPE_WORD:
				break;

			case gpr::CHUNK_TYPE_COMMENT:
			{
				if (!parseComment(chunk.get_comment_text()))
					continue;
				break;
			}
			case gpr::CHUNK_TYPE_WORD_ADDRESS:
			{
				parseWordAddressChunk(chunk.get_word(), chunk.get_address());
				break;
			}
		}
	}
}

void GCodeProgramParser::updateCurrentBlockAbsCoords()
{
	for (unsigned short i = 0; i < extrWorkPoints.whichCoordsSetInBlock.size(); ++i)
	{// updateBlockCoords(): If this coord is not set in this block, use the most recent for absolute coordinates.
		if (extrWorkPoints.whichCoordsSetInBlock[i] != 0 || !isAbsoluteMode)
			continue;
		_blockCurrAbsCoordsCurr[i] = extrWorkPoints.lastAbsCoords[i];
	}
}

void GCodeProgramParser::updateLastAbsCoords()
{
	if (!isAbsoluteMode)
	{
		extrWorkPoints.lastAbsCoords += extrWorkPoints.blockCurrRelativeCoords;
	}
	else
	{
		extrWorkPoints.lastAbsCoords = _blockCurrAbsCoordsCurr;
	}
}

std::vector<Extrusion> GCodeProgramParser::createExtrusionData(const std::string& inputFile)
{
	Chronograph chronograph(__FUNCTION__, true);

	gpr::gcode_program gcodeProgram = importGCodeFromFile(inputFile);

	// Start with a one extruder, add new if we encounter T1+ block.
	std::vector<Extrusion> extruders{initializeExtruderData()};
	initializeCurrentExtruderReferences(extruders);

	// Reset main parser workpoint.
	extrWorkPoints.lastAbsCoords = {0,0,0,0};

	for (const gpr::block& block : gcodeProgram)
	{
		// Reset parser block workpoints.
		extrWorkPoints.whichCoordsSetInBlock = {0,0,0,0};
		extrWorkPoints.blockCurrRelativeCoords = {0,0,0,0};

		_blockStringCurr = block.to_string();

		parseChunks(block);

		if (!extrWorkPoints.areAnyCoordsSet())
			continue;

		updateCurrentBlockAbsCoords();
		updateLastAbsCoords();

		if (!isNewPathPoint())
			continue;

		_pathCurr.push_back(extrWorkPoints.lastAbsCoords);

		assert(++blockCount < blockCountLimit);
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

bool GCodeProgramParser::WorkPoints::areAnyCoordsSet() const
{
	return !whichCoordsSetInBlock.isZero();
}
