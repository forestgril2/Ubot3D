#pragma once

#include <string>

#include <json.hpp>

// for convenience
using json = nlohmann::json;

class SlicerParams
{
public:
	SlicerParams(std::string paramsPath);

private:
	std::vector<std::pair<std::string, std::vector<std::string>>> extractParameterGroups(const std::vector<std::string>& lines);

	json _params;
};
