#include "GCodeProgramGenerator.h"

#include <memory>

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

SharedExtrusions GCodeProgramGenerator::computeExtrusions(const SolidSurfaceModels& models,
														  const ExtrusionParamSets& params) const
{
	SharedExtrusions extrusions = primeExtrusions(models, params);

	// Get model data and assign it to different model groups in model collection:
	{
//		const TriangleData& mainModel = input.getMainModel();
//		const TriangleData& support   = input.getSupport()  ;
//		const TriangleData& rafts     = input.getRafts()    ;
//		const TriangleData& brim      = input.getBrim()     ;
//		const TriangleData& skirt     = input.getSkirt()    ;
	}

	const std::vector<Real> layerBottoms = computeLayerBottoms(models, params);

	// Generate Raft Extrusion
	// Generate a bottom Extrusion.
	// Generate DualExtrusions
	{
		// Keep generating DualExtrusions for each thick Extrusion layer, until reaching h=(maxHeight-topExtrHeight).
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
														const ExtrusionParamSets& params) const
{// Generate an ExtrusionId's and an empty Extrusion for the given params.
	return {};
}

std::vector<Real> GCodeProgramGenerator::computeLayerBottoms(const SolidSurfaceModels& models,
															 const ExtrusionParamSets& params) const
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
