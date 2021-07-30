#include "GCodeProgramProcessor.h"

#include <gcode_program.h>
#include <parser.h>
#include <cfloat>

#include <Extrusion.h>
#include <Helpers3D.h>

// That would be around 10GB GCode probably.
static const uint32_t kBlockCountLimit = 640'000'000;
static uint32_t blockCount = 0;
const std::set<std::string> GCodeProgramProcessor::_kPathAnnotations = {"_default",
																		"warstwa1",
																		"warstwa2",
																		"warstwa3",
																		"warstwa4"};

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

	_extrusionCurr->layers.push_back({_extruderPathsCurr->size(), previousLayerLastPoint.z()});
}

void GCodeProgramProcessor::setExtruder(const uint32_t extruderIndex)
{
	if (_extruders.end() == _extruders.find(extruderIndex))
	{
		_extruders[extruderIndex] = Extrusion();
	}

	if (_extrusionCurr == &_extruders[extruderIndex])
		return;

	// If we are switching extruders, we may currently assume, that the previous one is set off.
	// TODO: One day we may have two extruders working at the same time.
	setExtrusionOff(_extrusionCurr);
	// Update last paths.
	_extrusionCurr->numPaths = (*_extruderPathsCurr).size();
	setupCurrentExtruderReferences(extruderIndex);
}

bool GCodeProgramProcessor::isNewLayerComment(const std::string& comment)
{// First layer has bed level (0.0f), like all preparation steps. We return true only for layers 1+
	return
			// Cura format ;FLAVOR:Marlin
            (comment.starts_with("LAYER:") && (0 != comment.compare("LAYER:0")))
			||// Ubot3D format
            (comment.starts_with("layer"));
}

bool GCodeProgramProcessor::isNewPathPoint() const
{// Ignore movements without extrusion and identical extruder path coordinates.
	return _extrusionCurr->isActive &&
			(_pathCurr.size() == 0 || _extrWorkPoints.lastAbsCoords != _pathCurr.back());
}

bool GCodeProgramProcessor::isNewZLevel() const
{
	return !approximatelyEqual(_extrWorkPoints.lastAbsCoords.z(), _pathCurr.back().z(), FLT_MIN);
}

bool GCodeProgramProcessor::isExtruderFilamentPushed() const
{
	return definitelyGreaterThan(_extrWorkPoints.lastAbsCoords.w(), _pathCurr.back().w(), FLT_MIN);
}

void GCodeProgramProcessor::setupCurrentExtruderReferences(uint32_t extruderIndex)
{
	assert(_extruders.end() != _extruders.find(extruderIndex));
	_extrusionCurr = &_extruders[extruderIndex];
	assert(!_extrusionCurr->paths.empty());
	_extruderPathsCurr = &(_extrusionCurr->paths);
	assert(!_extruderPathsCurr->back().empty());
	// Set current extruder position to last point in last path.
	_blockCurrAbsCoordsCurr = _extruderPathsCurr->back().back();
	_newCoordsCurr = &_blockCurrAbsCoordsCurr;
}

bool GCodeProgramProcessor::isPathAnnotation(const std::string& s)
{
	return _kPathAnnotations.end() != _kPathAnnotations.find(s);
}

bool GCodeProgramProcessor::processComment(const std::string& comment)
{
	if (isNewLayerComment(comment))
	{
		pushNewLayer();
		return true;
	}
	else if (isPathAnnotation(comment))
	{
		_annotationCurr = &(*_kPathAnnotations.find(comment));
		return true;
	}

	return false;
}

void GCodeProgramProcessor::switchExtruderModes(const int value)
{
	switch (value)
	{
		case 0:
			setExtrusionOff(_extrusionCurr);
			return;
		case 1:
			setExtrusionOn(_extrusionCurr, _extrWorkPoints.lastAbsCoords);
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

		// Extruder/tool index choice.
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

std::map<uint32_t, Extrusion>& GCodeProgramProcessor::generateExtrusionsFromGCode(const std::string& inputFilePath)
{
	Chronograph chronograph(__FUNCTION__, true);

	const gpr::gcode_program gcodeProgram = importGCodeFromFile(inputFilePath);

	// Initialize one default extruder, add new if we encounter T1+ block.
	_extruders.clear();
	_extruders[0] = Extrusion();
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

		//=========================================
		// A lot of things may happen inside here!
		processChunks(block);
		//=========================================

		if (!_extrWorkPoints.areAnyCoordsSet())
			continue;

		updateCurrentBlockAbsCoords();
		updateLastAbsCoords();

		if (!isNewPathPoint())
			continue;

		if (_extrWorkPoints.isFilamentPulledBack() || !isExtruderFilamentPushed())
		{// In case extruder filament position is not increased, the current path is over.

			if (isNewZLevel())
			{// In such situation also new Z level is a signature of a new layer.
				pushNewLayer();
			}

			setExtrusionOff(_extrusionCurr);
		}

		_pathCurr.push_back(_extrWorkPoints.lastAbsCoords);

		assert(++blockCount < kBlockCountLimit);
	}

	if (!_pathCurr.empty())
	{
		_extrusionCurr->numPathPointsMax = std::max<size_t>(_extrusionCurr->numPathPointsMax, _pathCurr.size());
		std::cout << " #### adding subPath no. " << _extruderPathsCurr->size() -1<< ", _extruderCurr->numPathPointsMax: " << _extrusionCurr->numPathPointsMax << std::endl;

//		dumpSubPath(blockStringCurr, subPathCurr);

		std::swap(_extruderPathsCurr->back(), _pathCurr);
	}

	_extrusionCurr->numPathPointsMax = std::max<size_t>(_extrusionCurr->numPathPointsMax, _pathCurr.size());
	_extrusionCurr->numPaths = (*_extruderPathsCurr).size();

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

void GCodeProgramProcessor::setExtrusionOff(Extrusion* extrusion)
{// If we are setting extrusion off, swap created path with the empty one in path vector.
	if (!extrusion->isActive)
		return;
	extrusion->isActive = false;

	if (_pathCurr.empty())
		return;
	extrusion->numPathPointsMax = std::max<size_t>(extrusion->numPathPointsMax, _pathCurr.size());
	extrusion->numPaths = (*_extruderPathsCurr).size();

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

void GCodeProgramProcessor::setExtrusionOn(Extrusion* extrusion, const ExtrPoint& lastAbsCoords)
{// If we are setting extrusion on, add new (empty) path to path vector.
	if (extrusion->isActive)
		return;
	extrusion->isActive = true;
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
	return lastAbsCoords.w() <= 0;
}
