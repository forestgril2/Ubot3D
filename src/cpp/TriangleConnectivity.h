#pragma once

#include <QObject>
#include <set>
#include <memory>

class Triangle;
class Island;

using TriangleShared = std::shared_ptr<Triangle>;
using IslandShared = std::shared_ptr<Island>;
using Triangles = std::vector<TriangleShared>;

class Triangle
{
public:
	explicit Triangle(uint32_t positionIndex, const std::vector<uint32_t>& indices);

	uint32_t getIndex(uint32_t i); // only valid with 0, 1, 2

	bool isNeighbour(const Triangle& other);

	bool isAdded() const;
	void setAdded();

	uint32_t getNeighbourCount() const;
	Triangle& getNeighbour(uint32_t i);
	void addNeighbour(Triangle& neighbour);
	uint32_t getIndex(uint32_t i) const;

private:
	Triangles _neighbours; //changed to set
	uint32_t _positionIndex; // position of the first index of this triangle in the global vert array (which is in  3's)
	const std::vector<uint32_t>& _indices;
	bool _isAdded;
};


class Island
{
public:
	void recursiveAdd(Triangle& t);
	std::set<TriangleShared> get();

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
	void setupNeighbours();
	void createTriangles();
	std::vector<IslandShared> calculateIslands();

	std::vector<std::shared_ptr<Triangle>> _triangles;
	const std::vector<uint32_t>& _indices;
};
