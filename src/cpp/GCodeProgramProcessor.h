#pragma once

#include <string>

#include <CommonDefs.h>

namespace gpr
{
	class gcode_program;
	class block;
	class chunk;
	class addr;
};

using ExtrPath = Extrusion::Path;
using ExtrPoint = Extrusion::Point;
using ExtrLayer = Extrusion::Layer;

class GCodeProgramProcessor
{
public:
	std::vector<Extrusion> createExtrusionData(const std::string& inputFile);

private:
	static Extrusion initializeExtruderData();

	struct WorkPoints
	{
		bool areAnyCoordsSet() const;

		ExtrPoint lastAbsCoords = {0,0,0,0};
		ExtrPoint blockCurrRelativeCoords = {0,0,0,0};
		Vector4i whichCoordsSetInBlock = {0,0,0,0};
	} _extrWorkPoints;

	void setExtruder(const uint32_t extruderNumber);
	void initializeCurrentExtruderReferences(std::vector<Extrusion>& extruders);

	void processChunks(const gpr::block& block);
	void processWordAddress(const char word, const gpr::addr& address);
	void processExtrCoordSetting(const char coord, const Real value);
	bool processComment(const std::string& comment);

	void switchExtruderModes(const int value);
	void setExtrusionOff(Extrusion* extruder);
	void setExtrusionOn(Extrusion* extruder, const ExtrPoint& lastAbsCoords);
	void setAbsoluteModeOn();
	void setAbsoluteModeOff(ExtrPoint* blockCurrRelativeCoords);

	void updateCurrentBlockAbsCoords();
	void updateLastAbsCoords();

	bool isNewLayerComment(const std::string& comment);
	void pushNewLayer();

	bool isNewPathPoint() const;

	Extrusion* _extruderCurr;
	std::vector<ExtrPath>* _extruderPathsCurr;
	ExtrPath _pathCurr;
	ExtrPoint _blockCurrAbsCoordsCurr = {0,0,0,0};
	ExtrPoint* _newCoordsCurr = &_blockCurrAbsCoordsCurr;

	size_t _numPathPointsMax = 0;
	bool _isAbsoluteMode = true;

	static void dumpSubPath(const std::string& blockString, const ExtrPath& path);

	// TODO: This is for debug, may remove later:
	std::string _blockStringCurr = "";
};
