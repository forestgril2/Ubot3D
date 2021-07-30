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

std::shared_ptr<gpr::gcode_program> GCodeProgramGenerator::getProgram() const
{
	return _program;
}

std::shared_ptr<gpr::gcode_program> GCodeProgramGenerator::getProgram()
{
	return _program;
}

std::map<uint32_t, std::shared_ptr<Extrusion>> GCodeProgramGenerator::computeExtrusions(const SolidSurfaceModels& input) const
{
	std::map<uint32_t, std::shared_ptr<Extrusion>> extrusions;

	// Get model data and assign it to different model groups in model collection:
	{
		const TriangleData& mainModel = input.getMainModel();
		const TriangleData& support   = input.getSupport()  ;
		const TriangleData& rafts     = input.getRafts()    ;
		const TriangleData& brim      = input.getBrim()     ;
		const TriangleData& skirt     = input.getSkirt()    ;
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

std::shared_ptr<gpr::gcode_program> GCodeProgramGenerator::generateProgram(std::map<uint32_t, std::shared_ptr<Extrusion> >&& extrusions) const
{
	{// For every matching DualExtrusion layer index.
		// Keep generating gcode_program parts, starting from lowest layer, going up.
	}
}

}
