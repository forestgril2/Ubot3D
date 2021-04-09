#include "GCodeProgramProcessor.h"

#include <gcode_program.h>
#include <parser.h>

// That would be around 10GB GCode probably.
static const uint32_t blockCountLimit = 640'000'000;
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

void GCodeProgramProcessor::pushNewLayer()
{
	assert(_extruderPathsCurr->size() != 0);
	const ExtrPath& pathPrev = (*_extruderPathsCurr)[
							   _extruderPathsCurr->size() > 1 ? _extruderPathsCurr->size() -2 : 0
							   ];

	const ExtrPoint previousLayerLastPoint =
			(pathPrev.size() > 0) ? pathPrev[pathPrev.size() -1] : ExtrPoint{0,0,0,0};

	_extruderCurr->layers.push_back({_extruderPathsCurr->size(), previousLayerLastPoint.z()});
}

Extrusion GCodeProgramProcessor::initializeExtruderData()
{
	Extrusion newData;
	newData.paths.clear();
	newData.paths.push_back(ExtrPath({{0,0,0,0}}));
	// Layer bottom at start is just the bed level.
	newData.layers.push_back({0u, 0.0f});
	return newData;
}

void GCodeProgramProcessor::setExtruder(const uint32_t extruderIndex)
{
	if (_extruders.end() == _extruders.find(extruderIndex))
	{
		_extruders[extruderIndex]=initializeExtruderData();
	}

	if (_extruderCurr == &_extruders[extruderIndex])
		return;

	// If we are switching extruders, we may currently assume, that the previous one is set off.
	// TODO: One day we may have two extruders working at the same time.
	setExtrusionOff(_extruderCurr);
	setupCurrentExtruderReferences(extruderIndex);
}

bool GCodeProgramProcessor::isNewLayerComment(const std::string& comment)
{// First layer has bed level (0.0f), like all preparation steps. We return true only for layers 1+
	return
			// Cura format ;FLAVOR:Marlin
			(comment._Starts_with("LAYER:") && (0 != comment.compare("LAYER:0")))
			||// Ubot3D format
			(comment._Starts_with("layer"));
}

bool GCodeProgramProcessor::isNewPathPoint() const
{// Ignore movements without extrusion and identical extruder path coordinates.
	return _extruderCurr->isExtruding &&
			(_pathCurr.size() == 0 || _extrWorkPoints.lastAbsCoords != _pathCurr.back());
}

void GCodeProgramProcessor::setupCurrentExtruderReferences(uint32_t extruderIndex)
{
	assert(_extruders.end() != _extruders.find(extruderIndex));
	_extruderCurr = &_extruders[extruderIndex];
	assert(!_extruderCurr->paths.empty());
	_extruderPathsCurr = &(_extruderCurr->paths);
	assert(!_extruderPathsCurr->back().empty());
	// Set current extruder position to last point in last path.
	_blockCurrAbsCoordsCurr = _extruderPathsCurr->back().back();
	_newCoordsCurr = &_blockCurrAbsCoordsCurr;
}

bool GCodeProgramProcessor::processComment(const std::string& comment)
{
	if (!isNewLayerComment(comment))
		return false;

	pushNewLayer();
	return true;
}

void GCodeProgramProcessor::switchExtruderModes(const int value)
{
	switch (value)
	{
		case 0:
			setExtrusionOff(_extruderCurr);
			return;
		case 1:
			setExtrusionOn(_extruderCurr, _extrWorkPoints.lastAbsCoords);
			return;
		case 90:
			setAbsoluteModeOn();
			return;
		case 91:
			setAbsoluteModeOff(&_extrWorkPoints.blockCurrRelativeCoords);
			return;
		case 92:
			//setPosition()
			return;
	}
}

void GCodeProgramProcessor::processWordAddress(const char word, const gpr::addr& address)
{
	switch(word)
	{
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
			processExtrCoordSetting(word, Real(address.double_value()));
			break;

		// Extruder choice.
		case 'T':
			switch (address.tp())
			{
				case gpr::ADDRESS_TYPE_INTEGER:
					setExtruder(uint32_t(address.int_value()));
					break;
				case gpr::ADDRESS_TYPE_DOUBLE:
					assert(false);
					break;
			}
			break;


		default:
			break;
	}
}

void GCodeProgramProcessor::processExtrCoordSetting(const char coord, const Real value)
{
	switch(coord)
	{
		case 'X':
			_newCoordsCurr->x() = value;
			_extrWorkPoints.whichCoordsSetInBlock.x() = true;
			break;
		case 'Y':
			_newCoordsCurr->y() = value;
			_extrWorkPoints.whichCoordsSetInBlock.y() = true;
			break;
		case 'Z':
			_newCoordsCurr->z() = value;
			_extrWorkPoints.whichCoordsSetInBlock.z() = true;
			break;
		case 'E':
			// Extruded length.
			_newCoordsCurr->w() = value;
			_extrWorkPoints.whichCoordsSetInBlock.w() = true;
			break;
		default:
			assert(false);
	}
}

void GCodeProgramProcessor::processChunks(const gpr::block& block)
{
	for (const gpr::chunk& chunk : block)
	{
		switch(chunk.tp())
		{
			case gpr::CHUNK_TYPE_WORD_ADDRESS:
			{// Most interesting things happen here.
				processWordAddress(chunk.get_word(), chunk.get_address());
				break;
			}

			case gpr::CHUNK_TYPE_COMMENT:
			{
				if (!processComment(chunk.get_comment_text()))
					continue;

			}
			default:
				break;
		}
	}
}

void GCodeProgramProcessor::updateCurrentBlockAbsCoords()
{
	for (uint32_t i=0; i<_extrWorkPoints.whichCoordsSetInBlock.size(); ++i)
	{// If this coord is not set in this block, use the most recent for absolute coordinates.
		if (_extrWorkPoints.whichCoordsSetInBlock[i] != 0 || !_isAbsoluteMode)
			continue;
		_blockCurrAbsCoordsCurr[i] = _extrWorkPoints.lastAbsCoords[i];
	}
}

void GCodeProgramProcessor::updateLastAbsCoords()
{
	if (!_isAbsoluteMode)
	{
		_extrWorkPoints.lastAbsCoords += _extrWorkPoints.blockCurrRelativeCoords;
	}
	else
	{
		_extrWorkPoints.lastAbsCoords = _blockCurrAbsCoordsCurr;
	}
}

std::map<uint32_t, Extrusion>& GCodeProgramProcessor::createExtrusionData(const std::string& inputFilePath)
{
	Chronograph chronograph(__FUNCTION__, true);

	const gpr::gcode_program gcodeProgram = importGCodeFromFile(inputFilePath);

	// Initialize one default extruder, add new if we encounter T1+ block.
	_extruders.clear();
	_extruders[0]=initializeExtruderData();
	setupCurrentExtruderReferences(0);

	// Reset main parser workpoint.
	_extrWorkPoints.lastAbsCoords = {0,0,0,0};

	// TODO: Add a progress indicator.
	for (const gpr::block& block : gcodeProgram)
	{
		// Reset parser block workpoints.
		_extrWorkPoints.whichCoordsSetInBlock = {0,0,0,0};
		_extrWorkPoints.blockCurrRelativeCoords = {0,0,0,0};

		_blockStringCurr = block.to_string();

		// A lot of things may happen inside here!
		processChunks(block);

		if (!_extrWorkPoints.areAnyCoordsSet())
			continue;

		updateCurrentBlockAbsCoords();
		updateLastAbsCoords();

		if (!isNewPathPoint())
			continue;

		if (_extrWorkPoints.isFilamentPulledBack())
		{// In case extruder filament position gets negative, the current path is over.
			setExtrusionOff(_extruderCurr);
		}

		_pathCurr.push_back(_extrWorkPoints.lastAbsCoords);

		assert(++blockCount < blockCountLimit);
	}

	if (!_pathCurr.empty())
	{
		_numPathPointsMax = std::max<size_t>(_numPathPointsMax, _pathCurr.size());
		std::cout << " #### adding subPath no. " << _extruderPathsCurr->size() -1<< ", maxPointsInSubPath: " << _numPathPointsMax << std::endl;

//		dumpSubPath(blockStringCurr, subPathCurr);

		std::swap(_extruderPathsCurr->back(), _pathCurr);
	}

	_extruderCurr->numPathPointsMax = _numPathPointsMax;
	_extruderCurr->numPaths = (*_extruderPathsCurr).size();

	return _extruders;
}

void GCodeProgramProcessor::dumpSubPath(const std::string& blockString, const Extrusion::Path& path)
{
	std::ofstream pathFile("subPathDump.txt");
	for (const ExtrPoint& point : path)
	{
		pathFile << "[" << point.x() << "," << point.y() << "," << point.z() << "," << point.w() << "]" << std::endl;
	}
	pathFile << blockString << std::endl;
	pathFile.close();
}

void GCodeProgramProcessor::setExtrusionOff(Extrusion* extruder)
{// If we are setting extrusion off, swap created path with the empty one in path vector.
	if (!extruder->isExtruding)
		return;
	extruder->isExtruding = false;

	if (_pathCurr.empty())
		return;
	_numPathPointsMax = std::max<size_t>(_numPathPointsMax, _pathCurr.size());

//	dumpSubPath(blockStringCurr, subPathCurr);

	if (_pathCurr.size() < 2)
	{
		static bool wasSubpathOfSmallSizeAnnounced = false;
		if (!wasSubpathOfSmallSizeAnnounced)
		{
			std::cout << "### WARNING: subpath of size: " << _pathCurr.size() << std::endl;
			wasSubpathOfSmallSizeAnnounced = true;
		}

		_pathCurr.clear();
		return;
	}

	_extruderPathsCurr->push_back(_pathCurr);
	_pathCurr = ExtrPath();
}

void GCodeProgramProcessor::setExtrusionOn(Extrusion* extruder, const ExtrPoint& lastAbsCoords)
{// If we are setting extrusion on, add new (empty) path to path vector.
	if (extruder->isExtruding)
		return;
	extruder->isExtruding = true;
	_pathCurr.push_back(lastAbsCoords);
}

void GCodeProgramProcessor::setAbsoluteModeOn()
{
	if (_isAbsoluteMode)
		return;
	_isAbsoluteMode = true;
	_newCoordsCurr = &_blockCurrAbsCoordsCurr;
}

void GCodeProgramProcessor::setAbsoluteModeOff(ExtrPoint* blockCurrRelativeCoords)
{
	if (!_isAbsoluteMode)
		return;

	_isAbsoluteMode = false;
	_newCoordsCurr = blockCurrRelativeCoords;
}

bool GCodeProgramProcessor::WorkPoints::areAnyCoordsSet() const
{
	return !whichCoordsSetInBlock.isZero();
}

bool GCodeProgramProcessor::WorkPoints::isFilamentPulledBack() const
{
	return lastAbsCoords.w() < 0;
}
