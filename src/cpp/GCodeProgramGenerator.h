#pragma once

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

using LayerBottomsDict = std::map<ExtrusionId, std::vector<Real>>;

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
	static LayerBottomsDict computeLayerBottoms(const SolidSurfaceModels& models,
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
	 * @param layerTop
	 * @return
	 */
	static DualExtrusionData splitDualExtrusion(SharedExtrusion& extrusion,
												const SharedSurface& model,
												const SharedParams params,
												const float layerBottom,
												const float layerTop);

	static SharedExtrusions generateDualExtrusions(const DualExtrusionData& data);

	SharedGCode generateProgram(SharedExtrusions&& extrusions) const;

	SharedGCode _program;
};
}
