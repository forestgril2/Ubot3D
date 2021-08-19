#include "GCodeProgramGenerator.h"

#include <memory>
#include <ranges>

#include <gcode_program.h>

#include <Extrusion.h>
#include <TriangleGeometry.h>

#undef NDEBUG

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

std::set<Real> GCodeProgramGenerator::getSortedUniqueLayerLevels(const LayerLevelDict& layerDict)
{
	return std::accumulate(layerDict.begin(), layerDict.end(), std::set<Real>({}),
							[](std::set<Real>&& set, const auto& dictPair) {
							   const auto& [extrId, bottoms] = dictPair;
							   for (Real bottom : bottoms)
							   {
								   set.insert(bottom);
							   }
							   // TODO: Check, if this is correct.
							   return std::move(set);
	});
}

std::set<LayerRange> GCodeProgramGenerator::getLayerRanges(const std::set<Real>& uniqueLevels)
{
	// TODO:
	return {};
}

SharedExtrusions GCodeProgramGenerator::computeExtrusions(const SolidSurfaceModels& models,
														  const ExtrusionParamSets& params) const
{
	SharedExtrusions extrusions = primeExtrusions(models, params);
	ExtrusionsRanges extrRanges = computeExtrusionRanges(models, params, extrusions);

	static const Real bedLevel = 0.0f;
	Real layerBottom = bedLevel;

	auto extrRangesIt = extrRanges.begin();
	while (extrRangesIt != extrRanges.end())
	{// We will add new extrusions and ranges until we finish with the top range.
		auto& [layerRange, extrusionIds] = *extrRangesIt;

//		for (auto& [extrId, extrusion] : layerExtrusions)
//		{// Split all extrusions for this bottom to extrusions for different layer heights/different extruders.
//			const DualExtrusionData dualExtrData = splitDualExtrusion(extrusion, models, params(extrId), layerTop);
//			SharedExtrusions splitted = generateDualExtrusions(dualExtrData);
//			for (auto& extr : splitted)
//			{// Extend extrusions-ranges with extrusions splitted to different tools.
//				extrRanges.insert(extr);
//			}
//		}

		// Get all extrusions for this bottom sorted by ranges.
//		const ExtrusionsRanges layerRangeDict = getExtrusionRangeDict(layerExtrusions);
//		for (const auto& [range, extrIds] : layerRangeDict)
//		{// Create extrusions for this bottom level in order from lowest to highest LayerRange
//			{// The same thick layer height applies to all model groups.
//				// Generate separate DualExtrusions for all model groups
//				// - which will fall into different Extrusions and/or Annotations
//				// Merge Extrusions within groups/Annotations.
//			}
//		}
	}

	return extrusions;
}

SharedExtrusions GCodeProgramGenerator::primeExtrusions(const SolidSurfaceModels& models,
														const ExtrusionParamSets& params)
{// Generate ExtrusionIds and empty Extrusions for the given params,
 // including separate extrusions for each model top and bottom layers.

	// ??? - old approach -??? Get model data and assign it to different model groups in model collection:
	{
//		const TriangleData& mainModel = input.getMainModel();
//		const TriangleData& support   = input.getSupport()  ;
//		const TriangleData& rafts     = input.getRafts()    ;
//		const TriangleData& brim      = input.getBrim()     ;
//		const TriangleData& skirt     = input.getSkirt()    ;
	}

	// TODO:
	return {};
}

ExtrusionsRanges GCodeProgramGenerator::computeExtrusionRanges(const SolidSurfaceModels& models,
															  const ExtrusionParamSets& params,
															  const SharedExtrusions& extrusions)
{
	// TODO:

//	const std::set<Real> levels = getSortedUniqueLayerLevels(layerRangesDict);
//	// If the first level is not at 0, something is clearly wrong.
//	assert(0.0 == *uniqueLevels.begin());
//	const Real extrusionTop = *levels.rbegin();
//  SharedExtrusions layerExtrusions = getIntersectedExtrusions(extrusions, layerTop);
//	const std::set<LayerRange> layerRanges = getLayerRanges(levels);

	return {};
}

SharedExtrusions GCodeProgramGenerator::getIntersectedExtrusions(SharedExtrusions extrusions, Real planeZ)
{// Collect all Extrusions, which are sliced by this bottom level plane.
	// TODO:
	return {};
}

ExtrusionsRanges GCodeProgramGenerator::getExtrusionRangeDict(const SharedExtrusions& layerExtrusions)
{
	// TODO:
	return {};
}

SharedGCode GCodeProgramGenerator::generateProgram(SharedExtrusions&& extrusions) const
{
	{// For every matching DualExtrusion layer index.
		// Keep generating gcode_program parts, starting from lowest layer, going up.
	}

	// TODO:
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
