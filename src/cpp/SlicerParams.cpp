#include "SlicerParams.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <string_view>

static const std::string kParamLinePrefix = "    ";
static const std::string kParamSwitchPrefixLong = "--";
static const std::string kParamSwitchPrefixShort = "-";

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

		if (line.starts_with(kParamLinePrefix))
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

std::vector<std::string> SlicerParams::extractParamSwitchParts(const std::vector<std::string>& lineGroup)
{
	std::vector<std::string> paramSwitchParts;

	for (const auto& line : lineGroup)
	{
		std::string lineLeft = line.substr(kParamLinePrefix.size(), std::string::npos);
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
		static const std::string paramTypePrefix = "<";
		static const std::string paramTypePostfix = ">";
		if (paramLineWords[currWordIndex].starts_with(paramTypePrefix) &&
			paramLineWords[currWordIndex].ends_with(paramTypePostfix))
		{// We also have the param type.
			currPart.append(wordDelimiter).append(paramLineWords[currWordIndex]);
		}
		// The switch part ends here and is fully initialized.
	}

	return paramSwitchParts;
}

json SlicerParams::constructJsonParamSchema(const std::vector<std::string>& lineTokens)
{
	return json();
}


SlicerParams::SlicerParams(std::string cliParamsPath)
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
	const std::vector<std::string> lines(std::sregex_token_iterator(fileContents.begin(), fileContents.end(), delimiter, -1),
										 std::sregex_token_iterator());

	// Extract parameter groups.
	auto lineGroups = extractParameterGroups(lines);

	for (const auto& [groupName, lineGroup]: lineGroups)
	{
		const auto paramSwitchParts = extractParamSwitchParts(lineGroup);

		std::cout << " ### " << __FUNCTION__ << " PARAM SWITCH PARTS IN GROUP:" << std::endl;
		for(const auto& token : paramSwitchParts)
		{
			std::cout << token << std::endl;
		}
		std::cout << std::endl;

//		for (const auto& line : lineGroup)
//		{
//			const auto paramLineTokens = extractParamSwitchTokens(line);
//			const json param = constructJsonParamSchema(paramLineTokens);
//		}

//		_params[groupName] = getJsonParamSchema(getParamLineTokens())
		std::cout << " ### " << __FUNCTION__ << " next group:" << groupName << std::endl;

		for (const auto& line: lineGroup)
		{
			std::cout << line << std::endl;
		}
	}

//	stream >> _params;
}
