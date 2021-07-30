#include "GCodeProgramGenerator.h"

#include <memory>

#include <gcode_program.h>

#include <Extrusion.h>
#include <TriangleGeometry.h>

namespace Slicer
{

GCodeProgramGenerator::GCodeProgramGenerator(const SolidSurfaceModels& input)
{
	// Generate Extrusions and convert them to GCode program.
	_program = generateProgram(computeExtrusions(input));
}

SharedGCode GCodeProgramGenerator::getProgram() const
{
	return _program;
}

SharedGCode GCodeProgramGenerator::getProgram()
{
	return _program;
}

SharedExtrusions GCodeProgramGenerator::computeExtrusions(const SolidSurfaceModels& input) const
{
	SharedExtrusions extrusions;

	// Get model data and assign it to different model groups in model collection:
	{
//		const TriangleData& mainModel = input.getMainModel();
//		const TriangleData& support   = input.getSupport()  ;
//		const TriangleData& rafts     = input.getRafts()    ;
//		const TriangleData& brim      = input.getBrim()     ;
//		const TriangleData& skirt     = input.getSkirt()    ;
	}
	const float maxHeight = input.getMaxheight();

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

SharedGCode GCodeProgramGenerator::generateProgram(SharedExtrusions&& extrusions) const
{
	{// For every matching DualExtrusion layer index.
		// Keep generating gcode_program parts, starting from lowest layer, going up.
	}
}

const SharedSurfaces& SolidSurfaceModels::operator()() const
{
	return _surfaces;
}

}
