#pragma once

#include <vector>
#include <list>

#include <CommonDefs.h>
using Vec3 = Vertex;

class PolygonTriangulation
{
public:
	PolygonTriangulation(const std::vector<Vec3>& vertices,
						 const std::list<std::vector<uint32_t>>& nodeRings);
};
