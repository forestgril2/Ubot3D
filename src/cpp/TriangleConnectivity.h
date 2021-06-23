#pragma once

#include <QObject>
#include <ostream>
#include <set>
#include <memory>

#include <Edge.h>

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
	std::vector<uint32_t> getVertexIndices() const;

	const std::set<Edge>& getBoundaryEdges() const;

	// An edge is denoted by a vertex opposite to it, but we specify it with two adjacent vertices.
	bool specifyInteriorEdge(const Edge& edge);

	friend std::ostream& operator<<(std::ostream& stream, const Triangle& triangle)
	{
		stream << " Triangle [_firstIndexPos, [indices], [_edges]]: [" << triangle._firstIndexPos
			   << ",[[" << triangle._vertexIndices[triangle._firstIndexPos]
			   << ", "  << triangle._vertexIndices[triangle._firstIndexPos +1]
			   << ", "  << triangle._vertexIndices[triangle._firstIndexPos +2] << "] " << std::endl;

		stream << ", boundary edges: " << std::endl << "[";
		for (const auto& edge : triangle._boundaryEdges)
		{
			stream << edge << ",";
		}
		stream << "]" << std::endl;
		return stream;
	}

	#ifndef NDEBUG
	friend class TriangleConnectivity;
	#endif

private:
	std::set<Edge> composeEdges();

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
	const std::set<Edge>& getBoundaryEdges() const;
	std::vector<uint32_t> getTriangleIndices() const;

private:
	Triangles _triangles = {};
	std::set<Edge> _boundaryEdges = {};
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
	void specifyCommonTriangleEdge(TriangleShared triangle, TriangleShared neighbour, const Edge& edge);
};
