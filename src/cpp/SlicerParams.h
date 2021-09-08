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
	json _params;
};
