#pragma once

#include <string>

#include <CommonDefs.h>

namespace gpr
{
	class block;
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
	static Extrusion initializeExtruderData();

	struct WorkingData
	{
		ExtrPoint lastAbsCoords = {0,0,0,0};
		ExtrPoint blockCurrRelativeCoords = {0,0,0,0};
		ExtrPoint whichCoordsSetInBlock = {0,0,0,0};
	};

	std::vector<ExtrLayer>& initGetCurrentLayers();
	void initializeCurrentExtruderReferences(std::vector<Extrusion>& extruders);
	void parseBlockChunk(const gpr::block& block, WorkingData& points);

	void setExtruder(const uint32_t extruderNumber);
	void setExtrusionOff(Extrusion* extruder);
	void setExtrusionOn(Extrusion* extruder, const ExtrPoint& lastAbsCoords);
	void setAbsoluteModeOn();
	void setAbsoluteModeOff(ExtrPoint* blockCurrRelativeCoords);

	void pushNewLayer();
	bool isNewLayerComment(const std::string& comment);

	Extrusion* _extruderCurr;
	std::vector<ExtrPath>* _extruderPathsCurr;
	ExtrPath _pathCurr;
	ExtrPoint _blockCurrAbsCoordsCurr = {0,0,0,0};
	ExtrPoint* _newCoordsCurr = &_blockCurrAbsCoordsCurr;

	size_t numPathPointsMax = 0;
	bool isAbsoluteMode = true;

	static void dumpSubPath(const std::string& blockString, const ExtrPath& path);
	// TODO: This is for debug, remove:
	std::string _blockStringCurr = "";
};
