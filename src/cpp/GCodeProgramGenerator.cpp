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
	}

	// Generate Extrusions
	{
		// Generate MultiLayers
		{
			// Generate a bottom MultiLayer structure.
			// Keep generating MultiLayers, until the top layer+(final multilayer height) <= maxHeight.
			// Generate a top MultiLayer structure.
		}

		// Regroup Multilayers to generate Extrusions
	}

	// Convert Extrusions to gcodeprogram
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

}
