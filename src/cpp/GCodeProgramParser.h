#pragma once

#include <string>

#include <CommonDefs.h>

namespace gpr
{
	class gcode_program;
};

using ExtrPath = Extrusion::Path;
using ExtrPoint = Extrusion::Point;
using ExtrLayer = Extrusion::Layer;

class GCodeProgramParser
{
public:
	std::vector<Extrusion> createExtrusionData(const std::string& inputFile);

private:
	static void dumpSubPath(const std::string& blockString, const ExtrPath& path);

	void setExtrusionOff(Extrusion* extruder);
	void setExtrusionOn(Extrusion* extruder, const ExtrPoint& lastAbsCoords);
	void setAbsoluteModeOn();
	void setAbsoluteModeOff(ExtrPoint* blockCurrRelativeCoords);
	void setExtruder(const uint32_t extruderNumber);
	void pushNewLayer();

	static Extrusion initializeExtruderData();

	bool isNewLayerComment(const std::string& comment);


	Extrusion* _extruderCurr;
	std::vector<ExtrPath>* _extruderPathsCurr;
	ExtrPath _pathCurr;
	ExtrPoint _blockCurrAbsCoordsCurr = {0,0,0,0};
	ExtrPoint* _newCoordsCurr = &_blockCurrAbsCoordsCurr;

	size_t numPathPointsMax = 0;
	bool isAbsoluteMode = true;


	// TODO: This is for debug, remove:
	std::string _blockStringCurr = "";
};
