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

class Triangle
{
public:
	explicit Triangle(uint32_t index, const std::vector<uint32_t>& indices);

	bool isNeighbour(const Triangle& other);

	bool isAdded() const;
	void setAdded();

	uint32_t getNeighbourCount() const;
	TrianglesWeak& getNeighbours();
	void addNeighbour(TriangleShared& neighbour);
	uint32_t getVertexIndex(uint32_t i) const;

private:
	TrianglesWeak _neighbours; //changed to set
	uint32_t _firstIndexPos; // position of the first index of this triangle in the global vert array (which is in  3's)
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
	std::vector<TriangleIsland> operator()();

private:
	void setupTriangleNeighbours();
	void createTriangles();
	std::vector<TriangleIsland> calculateIslands();

	Triangles _triangles;
	const std::vector<uint32_t>& _indices;
};
