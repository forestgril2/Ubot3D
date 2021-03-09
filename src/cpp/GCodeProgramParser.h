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

class GCodeProgramParser
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
	} extrWorkPoints;

	void setExtruder(const uint32_t extruderNumber);
	void initializeCurrentExtruderReferences(std::vector<Extrusion>& extruders);

	void parseChunks(const gpr::block& block);
	bool parseComment(const std::string& comment);
	void parseWordAddressChunk(const char word, const gpr::addr& address);
	void handleExtrCoordSetting(const char coord, const Real value);

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

	size_t numPathPointsMax = 0;
	bool isAbsoluteMode = true;

	static void dumpSubPath(const std::string& blockString, const ExtrPath& path);
	// TODO: This is for debug, remove:
	std::string _blockStringCurr = "";
};
