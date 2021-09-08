#include "SlicerParams.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <string_view>

std::vector<std::pair<std::string, std::vector<std::string>>> SlicerParams::extractParameterGroups(const std::vector<std::string>& lines)
{
	std::vector<std::pair<std::string, std::vector<std::string>>> lineGroups;
	std::string currentGroupName = "Basic IO config";
	lineGroups.push_back({currentGroupName, {}});
	const std::string firstLineUsageHintPart = "[ OPTIONS ] [ file.stl ] [ file2.stl ] ...";
	const std::string paramLinePrefix = "    ";
	const std::string groupNameLinePrefix = "  ";
	const std::string groupNameLinePostfix = ":";
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

		if (line.starts_with(paramLinePrefix))
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
	std::regex delimiter("\n");
	const std::vector<std::string> lines(std::sregex_token_iterator(fileContents.begin(), fileContents.end(), delimiter, -1),
										 std::sregex_token_iterator());

	// Extract parameter groups.
	auto lineGroups = extractParameterGroups(lines);

	for (const auto& [groupName, lineGroup]: lineGroups)
	{
		std::cout << " ### " << __FUNCTION__ << " next group:" << groupName << std::endl;

		for (const auto& line: lineGroup)
		{
			std::cout << line << std::endl;
		}
	}

//	stream >> _params;
}
