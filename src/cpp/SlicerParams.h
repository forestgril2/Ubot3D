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
	static std::vector<std::pair<std::string, std::vector<std::string>>> extractParameterGroups(const std::vector<std::string>& lines);
	static std::vector<std::string> extractParamLineTokens(const std::string& line);
	static json constructJsonParamSchema(const std::vector<std::string>& lineTokens);
	static std::vector<std::string> extractParamSwitchParts(const std::vector<std::string>& lineGroup);


	json _params;
};
