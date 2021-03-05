#pragma once

#include <string>

#include <CommonDefs.h>

namespace gpr
{
	class gcode_program;
};

class GCodeProgramParser
{
public:
	std::vector<Extrusion> createExtrusionData(const std::string& inputFile);

private:
	static void dumpSubPath(const std::string& blockString, const Extrusion::Path& path);

	void setExtrusionOff(Extrusion* extruder);
	void setExtrusionOn(Extrusion* extruder, const Extrusion::Point& lastAbsCoords);
	void setAbsoluteModeOn(Extrusion::Point& blockCurrAbsCoords);
	void setAbsoluteModeOff(Extrusion::Point& blockCurrRelativeCoords);
	void setExtruder(const uint32_t extruderNumber);
	void pushNewLayer();

	static Extrusion initializeExtruderData();

	bool isNewLayerComment(const std::string& comment);

	Extrusion* extruderCurr;

	std::vector<Extrusion::Path>* extruderPaths;
	Extrusion::Path pathCurr;

	Extrusion::Point blockCurrAbsCoords = {0,0,0,0};
	Extrusion::Point* newCoordsCurr = &blockCurrAbsCoords;

	std::string blockStringCurr;

	size_t numPathPointsMax = 0;
	bool isAbsoluteMode = true;
};
