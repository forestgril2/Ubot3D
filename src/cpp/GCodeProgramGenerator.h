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

namespace Slicer
{


class SolidSurfaceModels
{
public:
	const TriangleData& getMainModel() const;
	const TriangleData& getSupport()   const;
	const TriangleData& getRafts()     const;
	const TriangleData& getBrim()      const;
	const TriangleData& getSkirt()     const;

	float getMaxheight() const;
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

	std::shared_ptr<gpr::gcode_program> getProgram() const;
	std::shared_ptr<gpr::gcode_program> getProgram();

private:
	std::map<uint32_t, std::shared_ptr<Extrusion>> computeExtrusions(const SolidSurfaceModels& input) const;
	std::shared_ptr<gpr::gcode_program> generateProgram(std::map<uint32_t, std::shared_ptr<Extrusion>>&& extrusions) const;

	std::shared_ptr<gpr::gcode_program> _program;
};
}
