#pragma once

#include <memory>
#include <vector>

#include <Extrusion.h>
#include <Layer.h>

namespace gpr
{
	class gcode_program;
	class block;
	class chunk;
	class addr;
};

namespace Slicer
{


class GCodeProgramGeneratorParams;

struct DualExtrusion
{
	Extrusion extr0;
	Extrusion extr1;

	Real height;
};

class GCodeProgramGenerator
{
public:
	GCodeProgramGenerator(const GCodeProgramGeneratorParams& params);

	std::shared_ptr<gpr::gcode_program> getProgram() const;
	std::shared_ptr<gpr::gcode_program> getProgram();

private:
	std::shared_ptr<gpr::gcode_program> _program;
};
}
