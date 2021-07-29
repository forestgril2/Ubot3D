#pragma once

#include <set>
#include <string>

#include <CommonDefs.h>
#include <Extrusion.h>

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
	std::map<uint32_t, Extrusion>& createExtrusionData(const std::string& inputFilePath);

private:
	struct WorkPoints
	{
		bool areAnyCoordsSet() const;
		bool isFilamentPulledBack() const;

		ExtrPoint lastAbsCoords = {0,0,0,0};
		ExtrPoint blockCurrRelativeCoords = {0,0,0,0};
		Vector4i whichCoordsSetInBlock = {0,0,0,0};
	} _extrWorkPoints;

	void setExtruder(const uint32_t extruderIndex);
	void setupCurrentExtruderReferences(uint32_t extruderIndex);

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
	bool isNewZLevel() const;
	bool isExtruderFilamentPushed() const;
	static bool isPathAnnotation(const std::string& s);

	std::map<uint32_t, Extrusion> _extruders;
	Extrusion* _extruderCurr = nullptr;
	std::vector<ExtrPath>* _extruderPathsCurr;
	ExtrPath _pathCurr;
	ExtrPoint _blockCurrAbsCoordsCurr = {0,0,0,0};
	ExtrPoint* _newCoordsCurr = &_blockCurrAbsCoordsCurr;
	const std::string* _annotationCurr = &(*_kPathAnnotations.begin());

	bool _isAbsoluteMode = true;

	static void dumpSubPath(const std::string& blockString, const ExtrPath& path);

	const static std::set<std::string> _kPathAnnotations;

	// TODO: This is for debug, may remove later:
	std::string _blockStringCurr = "";
};
