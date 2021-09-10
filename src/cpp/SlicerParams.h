#pragma once

#include <string>

#include <json.hpp>

// for convenience
using json = nlohmann::json;

class SlicerParams
{
public:
	SlicerParams(const std::string& paramsPath);

private:
	static std::vector<std::pair<std::string, std::vector<std::string>>> extractParameterGroups(const std::vector<std::string>& lines);
	static std::vector<std::string> extractParamLineTokens(const std::string& line);
	static json constructJsonParamSchema(const std::vector<std::string>& lineTokens);
	static std::vector<std::string> extractParamSwitchLineParts(const std::vector<std::string>& lineGroup);
	static std::vector<json> extractGroupParams(const std::vector<std::string>& groupLines,
												const std::vector<std::string>& paramSwitchLineParts);
	static std::string extractLongParamSwitch(const std::string& paramSwitchLinePart);
	static std::string extractShortParamSwitch(const std::string& paramSwitchLinePart);
	static std::string extractParamType(const std::string& paramSwitchLinePart);
	static std::string truncateParamDescriptionWhitespaces(const std::string& paramDescriptionLine);

	json _params;
};
