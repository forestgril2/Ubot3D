#pragma once

#include <QObject>
#include <set>
#include <memory>

class Triangle;
class TriangleIsland;

using TriangleShared = std::shared_ptr<Triangle>;
using IslandShared = std::shared_ptr<TriangleIsland>;
using Triangles = std::vector<TriangleShared>;

class Triangle
{
public:
	explicit Triangle(uint32_t index, const std::vector<uint32_t>& indices);

	bool isNeighbour(const Triangle& other);

	bool isAdded() const;
	void setAdded();

	uint32_t getNeighbourCount() const;
	Triangles& getNeighbours();
	void addNeighbour(Triangle& neighbour);
	uint32_t getVertexIndex(uint32_t i) const;

private:
	Triangles _neighbours; //changed to set
	uint32_t _firstIndexPos; // position of the first index of this triangle in the global vert array (which is in  3's)
	const std::vector<uint32_t>& _vertexIndices;
	bool _isAdded;
};


class TriangleIsland
{
public:
	void recursiveAdd(Triangle& triangle);
	std::set<TriangleShared> getTriangles();

private:
	void addAndSetAdded(Triangle& t);
	std::set<TriangleShared> _triangles;
};

class TriangleConnectivity
{
public:
	TriangleConnectivity(const std::vector<uint32_t>& indices);
	std::vector<IslandShared> operator()();

private:
	void setupTriangleNeighbours();
	void createTriangles();
	std::vector<IslandShared> calculateIslands();

	std::vector<std::shared_ptr<Triangle>> _triangles;
	const std::vector<uint32_t>& _indices;
};
