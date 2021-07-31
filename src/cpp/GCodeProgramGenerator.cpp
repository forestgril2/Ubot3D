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

	assert(0 == uniqueBottoms[0]);
	float prevBottom = 0.0f;
	for (Real layerBottom : uniqueBottoms | std::views::drop(1))
	{// Keep generating DualExtrusions until reaching top.
		SharedExtrusions layerExtrusions = getIntersectedExtrusions(extrusions, layerBottom);
		const LayerLevelDict layerTopsDict = getLayerExtrusionTopsDict(layerExtrusions, layerBottom);
		const std::vector<Real> layerTops = getSortedUniqueLayerLevels(layerTopsDict);


		{// The same thick layer height applies to all model groups.
			// Generate separate DualExtrusions for all model groups
			// - which will fall into different Extrusions and/or Annotations
			// Merge Extrusions within groups/Annotations.
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

LayerLevelDict GCodeProgramGenerator::getLayerExtrusionTopsDict(const SharedExtrusions& layerExtrusions, Real layerBottom)
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
