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

	// Generate slices
	{
		// Generate a bottom SliceStructure.
		// Keep generating SliceStructures, until the top slice is higher, than the max.
		// Generate a top SliceStructure.
	}

	// Convert slices to gcodeprogram
	{
		// Keep generating gcode_program parts, starting from lowest slice.
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
