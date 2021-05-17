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
using Edge = std::pair<uint32_t, uint32_t>;

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
	std::set<uint32_t> getVertexIndices() const;

	const std::set<Edge>& getBoundaryEdges() const;

	// An edge is denoted by a vertex opposite to it, but we specify it with two adjacent vertices.
	void specifyInteriorEdge(const std::pair<uint32_t, uint32_t>& edge);

private:
	std::set<Edge> calculateEdges();

	TrianglesSet _neighbours; // TODO: may profile for speed with a std::vector<uint32_t>, as recursiveAdd() discards duplicates anyway.
	uint32_t _firstIndexPos; // position of the first index of this triangle in the vertex array (which is in  3's)
	const std::vector<uint32_t>& _vertexIndices;
	const std::set<Edge> _edges;

	// If two vertices aren't shared by the same neighbour triangle - they form a TriangleIsland boundary. Otherwise, they are interior.
	std::set<Edge> _boundaryEdges;

	bool _isAdded = false;

};


class TriangleIsland
{
public:
	TriangleIsland(TriangleShared& initialTriangle, uint32_t& trianglesLeft);
	Triangles& getTriangles();
	const Triangles& getTriangles() const;
	const std::set<Edge>& getEdges() const;
	std::vector<uint32_t> getTriangleIndices() const;

private:
	void calculateBoundaries();

	Triangles _triangles;
	std::set<Edge> _edges;
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

	std::map<uint32_t, std::set<TriangleShared> > getTrianglesAtVertices() const;

	Triangles _triangles;
	const std::vector<uint32_t>& _indices;
	void composeTriangleNeighbourhoodAtVertex(TriangleShared thisTriangle, TriangleShared neighbour, uint32_t vertexIndex,
											  std::map<TriangleShared, std::pair<Edge, bool> >& edgesAtNewNeighbours);
};
