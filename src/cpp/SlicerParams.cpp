#include "SlicerParams.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <string_view>

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

	std::regex delimiter("\n");
	std::vector<std::string> tokens(std::sregex_token_iterator(fileContents.begin(), fileContents.end(), delimiter, -1),
								   std::sregex_token_iterator());

	for (const auto& token : tokens)
	{
		std::cout << " ### TOKEN:" << std::endl;
		std::cout << token << std::endl;
	}

//	stream >> _params;
}
