#include "GCodeProgramGenerator.h"

#include <memory>
#include <ranges>

#include <gcode_program.h>

#include <Extrusion.h>
#include <TriangleGeometry.h>

namespace Slicer
{

GCodeProgramGenerator::GCodeProgramGenerator(const SolidSurfaceModels& models, const ExtrusionParamSets& params)
{
	// Generate Extrusions and convert them to GCode program.
	_program = generateProgram(computeExtrusions(models, params));
}

SharedGCode GCodeProgramGenerator::getProgram() const
{
	return _program;
}

SharedGCode GCodeProgramGenerator::getProgram()
{
	return _program;
}

std::vector<Real> GCodeProgramGenerator::getSortedUniqueLayerLevels(const LayerLevelDict& layerDict)
{
	std::set<Real> uniqueBottoms = std::accumulate(layerDict.begin(), layerDict.end(), std::set<Real>({}),
												[](std::set<Real>&& set, const auto& dictPair) {
												   const auto& [extrId, bottoms] = dictPair;
												   for (Real bottom : bottoms)
												   {
													   set.insert(bottom);
												   }
												   // TODO: Check, if this is correct.
												   return std::move(set);
												});
	return std::vector<Real>(uniqueBottoms.begin(), uniqueBottoms.end());
}

SharedExtrusions GCodeProgramGenerator::computeExtrusions(const SolidSurfaceModels& models,
														  const ExtrusionParamSets& params) const
{
	SharedExtrusions extrusions = primeExtrusions(models, params);
	const LayerLevelDict layerBottomsDict = computeLayerBottoms(models, params, extrusions);
	std::vector<Real> uniqueBottoms = getSortedUniqueLayerLevels(layerBottomsDict);

	static const Real bedLevel = 0.0f;
	assert(bedLevel == uniqueBottoms[0]);
	float prevBottom = bedLevel;
	for (const Real bottom : uniqueBottoms | std::views::drop(1))
	{// Keep generating extrusions until reaching top.
		SharedExtrusions layerExtrusions = getIntersectedExtrusions(extrusions, bottom);
		for (auto& [extrId, extrusion] : layerExtrusions)
		{// Split all extrusions for this bottom to extrusions for different layer heights/different extruders.
			const DualExtrusionData dualExtrData = splitDualExtrusion(extrusion, models, params(extrId), bottom);
			SharedExtrusions splitted = generateDualExtrusions(dualExtrData);
			for (auto& extr : splitted)
			{// Extend layer extrusions with extrusions splitted to different tools.
				layerExtrusions.insert(extr);
			}
		}
		// Get all extrusions for this bottom sorted by ranges.
		const LayerRangeDict layerRangeDict = getExtrusionRangeDict(layerExtrusions);

		for (const auto& [range, extrIds] : layerRangeDict)
		{// Create extrusions for this bottom level in order frow lowest to highest LayerRange
			{// The same thick layer height applies to all model groups.
				// Generate separate DualExtrusions for all model groups
				// - which will fall into different Extrusions and/or Annotations
				// Merge Extrusions within groups/Annotations.
			}
		}
	}
	// Generate a top Extrusion

	return extrusions;
}

SharedExtrusions GCodeProgramGenerator::primeExtrusions(const SolidSurfaceModels& models,
														const ExtrusionParamSets& params)
{// Generate an ExtrusionId's and an empty Extrusion for the given params,
 // including separate extrusions for each top and model bottom layers.

	// ??? - old approach -??? Get model data and assign it to different model groups in model collection:
	{
//		const TriangleData& mainModel = input.getMainModel();
//		const TriangleData& support   = input.getSupport()  ;
//		const TriangleData& rafts     = input.getRafts()    ;
//		const TriangleData& brim      = input.getBrim()     ;
//		const TriangleData& skirt     = input.getSkirt()    ;
	}
	return {};
}

LayerLevelDict GCodeProgramGenerator::computeLayerBottoms(const SolidSurfaceModels& models,
															const ExtrusionParamSets& params,
															const SharedExtrusions& extrusions)
{
	return {};
}

SharedExtrusions GCodeProgramGenerator::getIntersectedExtrusions(SharedExtrusions extrusions, Real planeZ)
{// Collect all Extrusions, which are sliced by this bottom level plane.
	return {};
}

LayerRangeDict GCodeProgramGenerator::getExtrusionRangeDict(const SharedExtrusions& layerExtrusions)
{
	return {};
}

SharedGCode GCodeProgramGenerator::generateProgram(SharedExtrusions&& extrusions) const
{
	{// For every matching DualExtrusion layer index.
		// Keep generating gcode_program parts, starting from lowest layer, going up.
	}

	return SharedGCode();
}

const SharedSurfaces& SolidSurfaceModels::operator()() const
{
	return _surfaces;
}

Real SolidSurfaceModels::getMaxheight() const
{
	return _maxHeight;
}

}
