#include "GCodeProgramGenerator.h"

#include <gcode_program.h>

GCodeProgramGenerator::GCodeProgramGenerator(const GCodeProgramGeneratorParams& params)
{
	// Get model data and assign it to different model groups in model collection:
	{
		// Main model
		// Support
		// Rafts
		// Brim
	}

	// Generate MultiLayers
	{
		// Generate a bottom MultiLayer structure.
		// Keep generating MultiLayers, until the top layer+(final multilayer height) <= maxHeight.
		// Generate a top MultiLayer structure.
	}

	// Convert layers to gcodeprogram
	{
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
