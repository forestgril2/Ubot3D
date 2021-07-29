#pragma once

#include <memory>
#include <vector>

namespace gpr
{
	class gcode_program;
	class block;
	class chunk;
	class addr;
};

class GCodeProgramGeneratorParams;
class SliceStructure;

class GCodeProgramGenerator
{
public:
	GCodeProgramGenerator(const GCodeProgramGeneratorParams& params);

	std::shared_ptr<gpr::gcode_program> getProgram() const;
	std::shared_ptr<gpr::gcode_program> getProgram();

private:
	std::shared_ptr<gpr::gcode_program> _program;
	std::vector<SliceStructure> _slices;
};
