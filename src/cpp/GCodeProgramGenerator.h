#pragma once

#include <map>
#include <memory>
#include <vector>

#include <json.hpp>

// for convenience
using json = nlohmann::json;

#include <Extrusion.h>
#include <Layer.h>

namespace gpr
{
	class gcode_program;
	class block;
	class chunk;
	class addr;
};

class TriangleData;


namespace Slicer
{

using ExtrusionId = std::string;
using SurfaceId = std::string;


struct ExtrusionParams
{
	ExtrusionId id;
	json params =
	{
		{"Infill" , {}},
		{"Outline", {}},
		{"Bottom" , {}},
		{"Top"	  , {}},
		{"Support", {}},
		{"Raft"	  , {}},
		{"Skirt"  , {}},
	};
};

using SharedGCode = std::shared_ptr<gpr::gcode_program>;
using SharedParams = std::shared_ptr<ExtrusionParams>;
using SharedSurface = std::shared_ptr<TriangleData>;
using SharedSurfaces = std::map<SurfaceId, SharedSurface>;
using SharedExtrusion = std::shared_ptr<Extrusion>;
using SharedExtrusions = std::map<ExtrusionId, SharedExtrusion>;

struct LayerRange
{
	Real top;
	Real bottom;
	bool operator<(const LayerRange& other);
};

using LayerLevelDict = std::multimap<ExtrusionId, std::vector<Real>>;
using LayerRangeDict = std::multimap<LayerRange, std::vector<ExtrusionId>>;

class SolidSurfaceModels
{
public:
	const SharedSurfaces& operator()() const;
	float getMaxheight() const;
private:
	SharedSurfaces _surfaces;
	float _maxHeight;
};

class DualExtrusionData
{
};

class ExtrusionParamSets
{
public:
	const SharedParams& operator()(const ExtrusionId& extrId) const;
private:
	SharedParams _params;
};

class GCodeProgramGenerator
{
public:
	GCodeProgramGenerator(const SolidSurfaceModels& models,
						  const ExtrusionParamSets& params);

	SharedGCode getProgram() const;
	SharedGCode getProgram();

private:
	SharedGCode generateProgram(SharedExtrusions&& extrusions) const;

	// Static methods
	SharedExtrusions computeExtrusions(const SolidSurfaceModels& models,
									   const ExtrusionParamSets& params) const;

	/**
	 * @brief primeExtrusions Generates a map of empty extrusions
	 * primed with their params sets to generated extrusion ids.
	 * @param models Model data for solid surfaces extruded in the GCode program.
	 * @param params Parameters for the extrusion.
	 * @return A mapping of empty extrusions to their ids.
	 */
	static SharedExtrusions primeExtrusions(const SolidSurfaceModels& models,
											const ExtrusionParamSets& params);
	static LayerLevelDict computeLayerBottoms(const SolidSurfaceModels& models,
											  const ExtrusionParamSets& params,
											  const SharedExtrusions& extrusions);

	/**
	 * @brief getIntersectedExtrusions Collects a subset of passed @param extrusions, which intersect with plane at @param planeZ.
	 * @param extrusions
	 * @param planeZ
	 * @return
	 */
	static SharedExtrusions getIntersectedExtrusions(SharedExtrusions extrusions, Real planeZ);

	/**
	 * @brief splitDualExtrusion Splits an Extrusion at a certain infill layer, and generates
	 * DualExtrusionData.
	 * @param extrusion
	 * @param model
	 * @param params
	 * @param layerBottom
	 * @return
	 */
	static DualExtrusionData splitDualExtrusion(SharedExtrusion& extrusion,
												const SolidSurfaceModels& models,
												const SharedParams params,
												const Real layerBottom);

	/**
	 * @brief getExtrusionRangeDict Creates a dictionary of layer renges within extrusions matched to ExtrusionId's.
	 * @param layerExtrusions
	 * @return
	 */
	static LayerRangeDict getExtrusionRangeDict(const SharedExtrusions& layerExtrusions);

	static SharedExtrusions generateDualExtrusions(const DualExtrusionData& data);

	// Members
	SharedGCode _program;
};
}
