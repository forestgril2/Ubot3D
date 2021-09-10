#include "SlicerParams.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <string_view>

static const std::string kParamLineWhitespacesPrefix = "    ";
static const std::string kParamSwitchPrefixLong = "--";
static const std::string kParamSwitchPrefixShort = "-";
static const std::string kParamTypePrefix = "<";
static const std::string kParamTypePostfix = ">";

static const std::string kParamGroupSchemaNameKey = "groupName";
static const std::string kParamGroupParamsKey     = "params";
static const std::string kParamCliSwitchLongKey   = "cliSwitchLong";
static const std::string kParamCliSwitchShortKey  = "cliSwitchShort";
static const std::string kParamTypeKey            = "valueType";
static const std::string kParamDescriptionKey     = "description";

static const json kParamGroupSchema =
{
	{kParamGroupSchemaNameKey, ""},
	{"groupTabText",           ""},
	{"description",            ""},
	{"isVisible",              true},
	{kParamGroupParamsKey,     {}},
};
static const json kParamSchema =
{
	{"name",                  ""},
	{"text",                  ""},
	{kParamCliSwitchLongKey,  ""},
	{kParamCliSwitchShortKey, ""},
	{kParamTypeKey,           ""},
	{"editFieldType",         ""},
	{kParamDescriptionKey,    ""},
	{"isVisible",             true},
	{"minValue",              ""},
	{"maxValue",              ""},
	{"defaultValue",          ""},
	{"stepValue",             ""},
	{"possibleValues",        {}},
};

std::vector<std::pair<std::string, std::vector<std::string>>> SlicerParams::extractParameterGroups(const std::vector<std::string>& lines)
{
	std::vector<std::pair<std::string, std::vector<std::string>>> lineGroups;
	std::string currentGroupName = "Basic IO config";
	lineGroups.push_back({currentGroupName, {}});
	static const std::string firstLineUsageHintPart = "[ OPTIONS ] [ file.stl ] [ file2.stl ] ...";
	static const std::string groupNameLinePrefix = "  ";
	static const std::string groupNameLinePostfix = ":";
	size_t currentPairIndex = 0;

	for (const auto& line : lines)
	{
		if (line.ends_with(firstLineUsageHintPart))
		{// Omit the command line usage hint.
			continue;
		}

		if (line.empty())
		{// Omit the command line usage hint.
			continue;
		}

		if (line.starts_with(kParamLineWhitespacesPrefix))
		{
			lineGroups[currentPairIndex].second.push_back(line);
			continue;
		}

		if (!line.starts_with(groupNameLinePrefix))
		{//Something is wrong.
			std::cout << " ### " << __FUNCTION__ << " ERROR: Unexpected line prefix, line: " << line << std::endl;
			exit(-1);
		}

		if (!line.ends_with(groupNameLinePostfix))
		{//Something is wrong.
			std::cout << " ### " << __FUNCTION__ << " ERROR: Unexpected line postfix, line: " << line << std::endl;
			exit(-1);
		}

		// Delete starting whitespaces and trailing colon from the new group name.
		currentGroupName = line
						   .substr(2, std::string::npos)
						   .substr(0, line.size() - (groupNameLinePrefix.size() + groupNameLinePostfix.size()));

		lineGroups.push_back({currentGroupName, {}});
		++currentPairIndex;
	}

	return lineGroups;
}

std::vector<std::string> SlicerParams::extractParamSwitchLineParts(const std::vector<std::string>& lineGroup)
{
	std::vector<std::string> paramSwitchParts;

	for (const auto& line : lineGroup)
	{
		std::string lineLeft = line.substr(kParamLineWhitespacesPrefix.size(), std::string::npos);
		const bool isLongPrefixFirst = lineLeft.starts_with(kParamSwitchPrefixLong);
		if (!isLongPrefixFirst && !lineLeft.starts_with(kParamSwitchPrefixShort))
			continue;

		// Tokenize by whitespaces.
		static const std::string wordDelimiter(" ");
		static const std::regex wordDelimiterRegex(wordDelimiter);
		const std::vector<std::string> paramLineWords(std::sregex_token_iterator(lineLeft.begin(), lineLeft.end(), wordDelimiterRegex, -1),
													  std::sregex_token_iterator());

		size_t currWordIndex = 0;
		paramSwitchParts.push_back(paramLineWords[currWordIndex]);
		std::string& currPart = paramSwitchParts.back();
		const std::string paramSwitchDelimiter = ",";
		if (currPart.ends_with(paramSwitchDelimiter))
		{// We also have the longer switch.
			if (isLongPrefixFirst)
			{// The longer prefix should be given as the second one).
				std::cout << " ### " << __FUNCTION__ << " ERROR: Unexpected param prefix order, line: " << line << std::endl;
				exit(-1);
			}
			currPart.append(wordDelimiter).append(paramLineWords[++currWordIndex]);
		}

		if  (1 == paramLineWords.size())
		{// Some parameter parts are longer that ususal,
		 // so a newline is inserted before the description.
		 // Thus the switch part ends here and is fully initialized.
			continue;
		}

		++currWordIndex;
		if (paramLineWords[currWordIndex].starts_with(kParamTypePrefix) &&
			paramLineWords[currWordIndex].ends_with(kParamTypePostfix))
		{// We also have the param type.
			currPart.append(wordDelimiter).append(paramLineWords[currWordIndex]);
		}
		// The switch part ends here and is fully initialized.
	}

//	std::cout << " ### " << __FUNCTION__ << " PARAM SWITCH PARTS IN GROUP:" << std::endl;
//	for(const auto& token : paramSwitchParts)
//	{
//		std::cout << token << std::endl;
//	}
//	std::cout << std::endl;

	return paramSwitchParts;
}

json SlicerParams::constructJsonParamSchema(const std::vector<std::string>& lineTokens)
{
	return json();
}


std::vector<json> SlicerParams::extractGroupParams(const std::vector<std::string>& groupLines,
												   const std::vector<std::string>& paramSwitchLineParts)
{
	std::vector<json> params;

	// For each param switch in group, initialize a new parameter.
	size_t currParamSwitchPartIndex = 0;
	const auto* currParamSwitchLinePart = &(paramSwitchLineParts[currParamSwitchPartIndex]);
	json* currParam = nullptr;

	for (const auto& line: groupLines)
	{
		const auto paramSwitchPartPos =
				line.find(*currParamSwitchLinePart, kParamLineWhitespacesPrefix.size());
		if (std::string::npos == paramSwitchPartPos)
		{// The line does not contain the CLI param switch, concatenate the line to the description.
			const std::string currentDescription = (*currParam)[kParamDescriptionKey];
			(*currParam)[kParamDescriptionKey] = currentDescription + truncateParamDescriptionWhitespaces(line);
			continue;
		}

		// The line does contain the CLI param switch, initialize a new parameter.
		params.push_back(kParamSchema);
		currParam = &params.back();
		(*currParam)[kParamCliSwitchLongKey]  = extractLongParamSwitch(*currParamSwitchLinePart);
		(*currParam)[kParamCliSwitchShortKey] = extractShortParamSwitch(*currParamSwitchLinePart);
		(*currParam)[kParamTypeKey]           = extractParamType(*currParamSwitchLinePart);
		(*currParam)[kParamDescriptionKey]    =
				truncateParamDescriptionWhitespaces(line.substr(paramSwitchPartPos + currParamSwitchLinePart->size()));

		if (currParamSwitchPartIndex == paramSwitchLineParts.size() -1)
			continue;
		currParamSwitchLinePart = &(paramSwitchLineParts[++currParamSwitchPartIndex]);
	}

	return params;
}

std::string SlicerParams::extractLongParamSwitch(const std::string& paramSwitchLinePart)
{
	const auto longPrefixPos = paramSwitchLinePart.find(kParamSwitchPrefixLong);
	const auto longPrefixEnd = paramSwitchLinePart.find(" ", longPrefixPos);
	return paramSwitchLinePart.substr(longPrefixPos, longPrefixEnd - longPrefixPos);
}

std::string SlicerParams::extractShortParamSwitch(const std::string& paramSwitchLinePart)
{
	const auto longPrefixPos = paramSwitchLinePart.find(kParamSwitchPrefixLong);
	const auto shortPrefixPos = paramSwitchLinePart.find(kParamSwitchPrefixShort);

	if (longPrefixPos > shortPrefixPos)
		return paramSwitchLinePart.substr(shortPrefixPos, paramSwitchLinePart.find(","));

	return "";
}

std::string SlicerParams::extractParamType(const std::string& paramSwitchLinePart)
{
	const auto paramTypePrefixPos = paramSwitchLinePart.find(kParamTypePrefix);
	if (paramTypePrefixPos == std::string::npos)
		return "";
	return paramSwitchLinePart.substr(paramTypePrefixPos, paramSwitchLinePart.find(kParamTypePostfix));
}

std::string SlicerParams::truncateParamDescriptionWhitespaces(const std::string& paramDescriptionLine)
{
	std::string s = paramDescriptionLine;
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());

	//Add one trailing space.
	return s + " ";
}

SlicerParams::SlicerParams(const std::string& cliParamsPath)
{
	std::ifstream fileStream(cliParamsPath);

	if (!fileStream.is_open())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR: cannot open file for reading: " << cliParamsPath << std::endl;
		return;
	}

	const std::string fileContents((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
	fileStream.close();

	// Read in parameter lines  - tokenize by newline delimiter.
	const std::regex delimiter("\n");
	const std::vector<std::string> paramLines(std::sregex_token_iterator(fileContents.begin(), fileContents.end(), delimiter, -1),
											  std::sregex_token_iterator());

	auto lineGroups = extractParameterGroups(paramLines);

	for (const auto& [groupName, groupLines]: lineGroups)
	{
		// Initialize a new json param group.
		_params.push_back(kParamGroupSchema);
		_params.back()[kParamGroupSchemaNameKey] = groupName;
		_params.back()[kParamGroupParamsKey] = extractGroupParams(groupLines, extractParamSwitchLineParts(groupLines));

//		for (const auto& line : lineGroup)
//		{
//			const auto paramLineTokens = extractParamSwitchTokens(line);
//			const json param = constructJsonParamSchema(paramLineTokens);
//		}

//		std::cout << " ### " << __FUNCTION__ << " next group:" << groupName << std::endl;
//		for (const auto& line: groupLines)
//		{
//			std::cout << line << std::endl;
//		}
	}

	std::ofstream jsonFile("C:\\Users\\Grzegorz Ilnicki\\Desktop\\JOB\\Ubot3D\\slicerParams.json");
	jsonFile << _params;

//	stream >> _params;
}
