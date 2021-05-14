#pragma once

#include <QObject>
#include <set>
#include <memory>

class Triangle;
class TriangleIsland;

using TriangleShared = std::shared_ptr<Triangle>;
using Triangles = std::vector<TriangleShared>;
using TrianglesList = std::vector<TriangleShared>;
using TrianglesSet = std::set<TriangleShared, bool(*)(const TriangleShared&, const TriangleShared&)>;

class Triangle
{
public:
	explicit Triangle(uint32_t pos, const std::vector<uint32_t>& indices);

	bool isNeighbour(const Triangle& other);
	bool operator<(const Triangle& other);

	bool isAdded() const;
	void setAdded();

	uint32_t getNeighbourCount() const;
	TrianglesSet& getNeighbours();
	std::pair<TrianglesSet::iterator, bool> addNeighbour(TriangleShared& neighbour);
	uint32_t getVertexIndex(uint32_t i) const;

private:
	TrianglesSet _neighbours; // TODO: may profile for speed with a std::vector<uint32_t>, as recursiveAdd() discards duplicates anyway.
	uint32_t _firstIndexPos; // position of the first index of this triangle in the vertex array (which is in  3's)
	std::vector<std::pair<uint32_t, uint32_t>> _interiorEdges; // If two vertices aren't shared by the same neighbour triangle - they form a TriangleIsland boundary. Otherwise, they are interior.
	const std::vector<uint32_t>& _vertexIndices;
	bool _isAdded = false;
};


class TriangleIsland
{
public:
	TriangleIsland(TriangleShared& initialTriangle, uint32_t& trianglesLeft);
	Triangles& getTriangles();
	const Triangles& getTriangles() const;
	std::vector<uint32_t> getTriangleIndices() const;

private:
	void calculateBoundaries();

	Triangles _triangles;
	std::vector<std::vector<uint32_t>> _boundaries; // Polygons on the edges of the island.
	uint32_t _myNumber;
	uint32_t _numFailedInsertions = 0;
};

class TriangleConnectivity
{
public:
	TriangleConnectivity(const std::vector<uint32_t>& indices);
	std::vector<TriangleIsland> calculateIslands();

private:
	void setupTriangleNeighboursAndEdges();
	void createTriangles();

	Triangles _triangles;
	const std::vector<uint32_t>& _indices;
};
