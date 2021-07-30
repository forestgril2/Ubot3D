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


class TriangleData;
using SharedGCode = std::shared_ptr<gpr::gcode_program>;
using SharedSurface = std::shared_ptr<TriangleData>;
using SharedSurfaces = std::map<std::string, SharedSurface>;
using SharedExtrusion = std::shared_ptr<Extrusion>;
using SharedExtrusions = std::map<std::string, SharedExtrusion>;

namespace Slicer
{

class SolidSurfaceModels
{
public:
	const SharedSurfaces& operator()() const;
	float getMaxheight() const;
private:
	SharedSurfaces _surfaces;
	float _maxHeight;
};

struct DualExtrusion
{
	Extrusion extr0;
	Extrusion extr1;

	Real height;
};

class GCodeProgramGenerator
{
public:
	GCodeProgramGenerator(const SolidSurfaceModels& input);

	SharedGCode getProgram() const;
	SharedGCode getProgram();

private:
	SharedExtrusions computeExtrusions(const SolidSurfaceModels& input) const;
	SharedGCode generateProgram(SharedExtrusions&& extrusions) const;

	SharedGCode _program;
};
}
