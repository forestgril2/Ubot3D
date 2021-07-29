#include "GCodeProgramGenerator.h"

#include <gcode_program.h>

#include <Extrusion.h>

namespace Slicer
{

GCodeProgramGenerator::GCodeProgramGenerator(const GCodeProgramGeneratorParams& params)
{
	// Get model data and assign it to different model groups in model collection:
	{
		// Main model
		// Support
		// Rafts
		// Brim
		// Skirt
	}

	// Generate Extrusions
	{
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
	}

	// Convert Extrusions to gcodeprogram
	{// For every matching DualExtrusion layer index.
		// Keep generating gcode_program parts, starting from lowest layer, going up.
	}
}

std::shared_ptr<gpr::gcode_program> GCodeProgramGenerator::getProgram() const
{
	return _program;
}

std::shared_ptr<gpr::gcode_program> GCodeProgramGenerator::getProgram()
{
	return _program;
}

}
