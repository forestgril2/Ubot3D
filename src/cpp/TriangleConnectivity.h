#pragma once

#include <QObject>
#include <set>
#include <memory>

class Triangle;
class TriangleIsland;

using TriangleShared = std::shared_ptr<Triangle>;
using TriangleWeak = std::weak_ptr<Triangle>;
using Triangles = std::vector<TriangleShared>;
using TrianglesWeak = std::vector<TriangleWeak>;
using TriangleSet = std::set<TriangleShared>;
using TrianglesWeakSet = std::set<TriangleWeak, bool(*)(const TriangleWeak& t1, const TriangleWeak& t2)>;

class Triangle
{
public:
	explicit Triangle(uint32_t pos, const std::vector<uint32_t>& indices);

	bool isNeighbour(const Triangle& other);

	bool isAdded() const;
	void setAdded();

	uint32_t getNeighbourCount() const;
	TrianglesWeakSet& getNeighbours();
	void addNeighbour(TriangleShared& neighbour);
	uint32_t getVertexIndex(uint32_t i) const;

private:

	TrianglesWeakSet _neighbours; // TODO: profile it with a std::vector<TriangleWeak>, as recursiveAdd() discards duplicates anyway.
	uint32_t _firstIndexPos; // position of the first index of this triangle in the vertex array (which is in  3's)
	const std::vector<uint32_t>& _vertexIndices;
	bool _isAdded;
};


class TriangleIsland
{
public:
	void recursiveAdd(TriangleShared& triangle);
	TriangleSet& getTriangles();

private:
	void addAndSetAdded(TriangleShared& triangle);
	TriangleSet _triangles;
};

class TriangleConnectivity
{
public:
	TriangleConnectivity(const std::vector<uint32_t>& indices);
	std::vector<TriangleIsland> calculateIslands();

private:
	void setupTriangleNeighbours();
	void createTriangles();

	Triangles _triangles;
	const std::vector<uint32_t>& _indices;
};
