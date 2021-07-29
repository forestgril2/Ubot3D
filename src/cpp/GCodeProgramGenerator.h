#pragma once

#include <memory>
#include <vector>
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

class MultiLayer
{
public:
	Layer thick;
	std::vector<Layer> thin;
};

class GCodeProgramGenerator
{
public:
	GCodeProgramGenerator(const GCodeProgramGeneratorParams& params);

	std::shared_ptr<gpr::gcode_program> getProgram() const;
	std::shared_ptr<gpr::gcode_program> getProgram();

private:
	std::shared_ptr<gpr::gcode_program> _program;
	std::vector<MultiLayer> _layers;
};
}
