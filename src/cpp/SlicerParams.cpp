#include "SlicerParams.h"

#include <fstream>
#include <sstream>
#include <iostream>

SlicerParams::SlicerParams(std::string cliParamsPath)
{
	std::ifstream fileStream(cliParamsPath);

	if (!fileStream.is_open())
	{
		std::cout << " ### " << __FUNCTION__ << " ERROR: cannot open file for reading: " << cliParamsPath << std::endl;
		return;
	}

	std::string fileContents((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
	fileStream.close();

//	std::stringstream str(stream);
	std::cout << fileContents;

//	stream >> _params;
}
