#pragma once

#include <string>

#include <CommonDefs.h>

namespace gpr
{
	class gcode_program;
};

class GCodeProgramParser
{
public:
	static ExtruderData createExtruderData(const std::string& inputFile);

private:
	static void dumpSubPath(const std::string& blockString, const ExtrPath& subPath);
};

