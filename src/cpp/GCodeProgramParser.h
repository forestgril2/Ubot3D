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
	ExtruderData createExtruderData(const std::string& inputFile);

private:
	static void dumpSubPath(const std::string& blockString, const ExtrPath& subPath);

	void setExtrusionOff();
	void setExtrusionOn(const ExtrPoint& lastAbsCoords);
	void setAbsoluteModeOn(ExtrPoint& blockCurrAbsCoords);
	void setAbsoluteModeOff(ExtrPoint& blockCurrRelativeCoords);

	ExtruderData data;
	std::vector<ExtrPath>& extruderPaths = data.extruderPaths;
	ExtrPath subPathCurr;

	ExtrPoint blockCurrAbsCoords = {0,0,0,0};
	ExtrPoint* newCoordsPtr = &blockCurrAbsCoords;

	std::string blockStringCurr;

	unsigned maxPointsInSubPath = 0;
	bool isAbsoluteMode = true;
	bool isExtruderOn = false;
};

