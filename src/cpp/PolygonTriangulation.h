#pragma once

#include <vector>
#include <list>

#include <CommonDefs.h>
#include <TriangleGeometry.h>

using Vec3 = Vertex;

class PolygonTriangulation
{
public:
	PolygonTriangulation(const std::vector<Vec3>& vertices,
						 const std::list<std::vector<uint32_t>>& boundaryNodeRings,
						 const float zLevel,
						 const Vec3& meshNormal);

	const TriangleGeometryData& getMesh() const { return _mesh; }
	const std::list<std::vector<uint32_t>>& getBorders() const { return _borders; }

private:
	TriangleGeometryData _mesh;
	std::list<std::vector<uint32_t>> _borders;
};
