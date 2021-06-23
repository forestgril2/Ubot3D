#include "TriangleConnectivity.h"
#include <iostream>     // std::cout
#include <algorithm>    // std::set_difference, std::sort
#include <vector>       // std::vector
#include <set>       // std::vector
#include <cmath>
#include <map>
#include <memory>

#include <Chronograph.h>
#include <Edge.h>

//#undef NDEBUG

// Most well-triangulated meshes should have around 6 triangles shared by one vertex. Let's reserve for 8.
static const uint32_t kNumTrianglesReservedPerVertex = 8;
// Most well-triangulated meshes should have around 12 neighbours triangles around. Let's reserve for 15.
static const uint32_t kNumNeighbourTriangles= 15;
static const uint32_t kMaxUnsigned = std::numeric_limits<uint32_t>::max();

static bool compareTriangles(const TriangleShared& t1, const TriangleShared& t2)
{
	return *t1 < *t2;
}

TriangleConnectivity::TriangleConnectivity(const std::vector<uint32_t>& indices) :
	_indices(indices)
{
	Chronograph chronograph(__FUNCTION__, true);
	assert(0 == _indices.size() % 3);
	createTriangles();
	setupTriangleNeighboursAndEdges();
}

void TriangleConnectivity::createTriangles()
{
	Chronograph chronograph(__FUNCTION__, true);
	_triangles.reserve(_indices.size()/3);
	for (uint32_t firstTriangleIndexPos=0; firstTriangleIndexPos<_indices.size(); firstTriangleIndexPos += 3)
	{// Jump through all first triangle vertex indices.
		_triangles.push_back(std::make_shared<Triangle>(firstTriangleIndexPos, _indices));
//		std::cout << " ### " << __FUNCTION__ << " firstTriangleIndexPos:" << " " << firstTriangleIndexPos << "" << std::endl;
	}
}

std::map<uint32_t, std::set<TriangleShared>> TriangleConnectivity::getTrianglesAtVertices() const
{
	// For every vertex index, assign triangles from triangle array
	// std::vector<TriangleShared> for triangles incident to that vertex.

	// TODO: Consider changing it to a pair of vectors for speed - one would
	// be a vector of unique indices (precalculated before) the other would
	// be this map, but changed to std::vector<std::vector<uint32_t>>.
	std::map<uint32_t, std::set<TriangleShared>> trianglesAtVertex;
	for (TriangleShared triangle : _triangles)
	{// Insert every triangle to collections incident to all its vertices.
		for (uint32_t vertexIndex : triangle->getVertexIndices())
		{
			auto [insertion, isInserted] = trianglesAtVertex[vertexIndex].insert(triangle);
			assert(isInserted); // Triangles should not repeat at this point.
		}
	}
	return trianglesAtVertex;
}

void TriangleConnectivity::composeTriangleNeighbourhoodAtVertex(TriangleShared thisTriangle,
																TriangleShared neighbour,
																uint32_t vertexIndex,
																std::map<TriangleShared, std::pair<Edge, bool>>& edgesAtNewNeighbours)
{
//	if (thisTriangle->_firstIndexPos == 168633 || neighbour->_firstIndexPos == 168633)
//	{
//		std::cout << " ### " << __FUNCTION__ << " culprit triangle:"  << std::endl << *thisTriangle << "," << "" << std::endl;
//		std::cout << " ### " << __FUNCTION__ << " culprit neighbour:" << std::endl << *neighbour << "," << "" << std::endl;
//	}
	const bool isNewNeighbour = thisTriangle->addNeighbour(neighbour).second;
	if (isNewNeighbour)
	{// Also add this one to the other triangle as neighbour.
		#ifndef NDEBUG
		const bool isAddedToTheOtherForTheFirstTime =
		#endif
		neighbour->addNeighbour(thisTriangle).second;
		assert(isAddedToTheOtherForTheFirstTime);

		// Remember the vertex index, at which this neighbour was found.
		#ifndef NDEBUG
		auto [neighbourThisRoundIt, isNewInThisTriangleRound] =
		#endif
		// We need a second addition of this neighbour in this triangle round to resolve this neighbourhood as a common edge.
		edgesAtNewNeighbours.insert({neighbour, {{vertexIndex, kMaxUnsigned}, /*isValid*/ false}});
		assert(isNewInThisTriangleRound);
		return;
	}

	auto neighbourIt = edgesAtNewNeighbours.find(neighbour);
	const bool neighbourExistsAtVertexInThisRound = (neighbourIt != edgesAtNewNeighbours.end());

	if (!neighbourExistsAtVertexInThisRound)
		return; // This means that the neighbourhood has already been resolved at the other "neighbour" triangle

	// If a vertex is already mapped to the "neighbour" for the second time in this triangle round,
	// report common edge resolution, by filling a valid common edge data.
	const uint32_t previousIndex = neighbourIt->second.first.first;
	neighbourIt->second.first = Edge(std::minmax(previousIndex, vertexIndex));
	neighbourIt->second.second = /* isValid */ true;
}

void TriangleConnectivity::specifyCommonTriangleEdge(TriangleShared triangle, TriangleShared neighbour, const Edge& edge)
{
	bool isSuccessful = true;
	isSuccessful &= triangle->specifyInteriorEdge(edge);
	isSuccessful &= neighbour->specifyInteriorEdge(edge);

	if (!isSuccessful)
	{
		std::cout << " ### " << __FUNCTION__ << ": " << *triangle << std::endl;
		std::cout << " ### " << __FUNCTION__ << ": " << *neighbour << std::endl;
	}
}

void TriangleConnectivity::setupTriangleNeighboursAndEdges()
{
	Chronograph chronograph(__FUNCTION__, true);

	// Every triangle has 3 vertices, but the same vertex may be common to many triangles.
	const std::map<uint32_t, std::set<TriangleShared>> trianglesAtVertex = getTrianglesAtVertices();

	for (TriangleShared masterTriangle : _triangles)
	{// For every triangle
		// Remember, which new triangles were added in round of this triangle - the "masterTriangle".
		// Map to them vertices and edges, at which they were added.
		std::map<TriangleShared, std::pair<Edge, bool>> edgesToNewNeighbours;

		for (const uint32_t vertexIndex : masterTriangle->getVertexIndices())
		{// For every vertex in master triangle.
			for (TriangleShared neighbour : trianglesAtVertex.at(vertexIndex))
			{// For every neighbour triangle incident to this vertex.
				if (neighbour == masterTriangle)
					continue; // Skip the masterTriangle at this vertex.

				// Setup neighbourhood of: masterTriangle and the neighbour at this vertex.
				composeTriangleNeighbourhoodAtVertex(masterTriangle, neighbour, vertexIndex, edgesToNewNeighbours);

				const auto edgeAtNeighbourIt = edgesToNewNeighbours.find(neighbour);
				if (edgeAtNeighbourIt == edgesToNewNeighbours.end())
					continue; // Neighbour is already resolved at the other triangle

				if (/*isEdgeValid*/ !edgeAtNeighbourIt->second.second)
					continue; // Edge is invalid.

				// The two triangles share a common interior edge - one, which is not an island boundary.
				const Edge& validCommonEdge = edgeAtNeighbourIt->second.first;
				#ifndef NDEBUG
//				if ((masterTriangle->_firstIndexPos == 168633 || neighbour->_firstIndexPos == 168633) && Edge{259782,259785} == validCommonEdge)
//				{
//					std::cout << " ### " << __FUNCTION__ << " SPECIFY EDGE triangle:"  << *masterTriangle << "," << "" << std::endl;
//					std::cout << " ### " << __FUNCTION__ << " SPECIFY EDGE neighbour:" << *neighbour << "," << "" << std::endl;
//				}
				#endif
				specifyCommonTriangleEdge(masterTriangle, neighbour, validCommonEdge);
			}
		}
	}
}

std::vector<TriangleIsland> TriangleConnectivity::calculateIslands()
{
	Chronograph chronograph(__FUNCTION__, false);
	std::vector<TriangleIsland> islands;
	uint32_t trianglesLeft = uint32_t(_triangles.size());
	for (TriangleShared& triangle : _triangles)
	{
		if(triangle->isAdded())
			continue;
		islands.push_back(TriangleIsland(triangle, trianglesLeft));
	}
	return islands;
}

uint32_t Triangle::getVertexIndex(uint32_t i) const
{
	assert(i<3);
	return _vertexIndices[_firstIndexPos + i];
}

std::vector<uint32_t> Triangle::getVertexIndices() const
{
	return {_vertexIndices[_firstIndexPos], _vertexIndices[_firstIndexPos +1], _vertexIndices[_firstIndexPos +2]};
}

const std::set<Edge>& Triangle::getBoundaryEdges() const
{
	return _boundaryEdges;
}

Triangle::Triangle(uint32_t pos, const std::vector<uint32_t>& indices) :
	_neighbours(compareTriangles),
	_firstIndexPos(pos),
	_vertexIndices(indices),
	_edges(composeEdges()),
	_boundaryEdges(_edges),
	_isAdded(false)
{
}

bool Triangle::isNeighbour(const Triangle& other)
{
	// Check if the other has a common vertex with this one.
	for (uint32_t i=0; i<3; ++i)
	{// For every vertex in this triangle.
		for (uint32_t j=0; j<3; ++j)
		{// And for every vertex in the other triangle.
			if (getVertexIndex(i) != other.getVertexIndex(j))
				continue;
			return true;
		}
	}
	return false;
}

bool Triangle::operator<(const Triangle& other)
{
	return _firstIndexPos < other._firstIndexPos;
}

bool Triangle::isAdded() const
{
	return _isAdded;
}

void Triangle::setAdded()
{
	_isAdded = true;
//	std::cout << " ### " << __FUNCTION__ << " _firstIndexPos:" << _firstIndexPos << "," << "" << std::endl;
}

uint32_t Triangle::getNeighbourCount() const
{
	return uint32_t(_neighbours.size());
}

TrianglesSet& Triangle::getNeighbours()
{
	return _neighbours;
}

std::pair<TrianglesSet::iterator, bool> Triangle::addNeighbour(TriangleShared& neighbour)
{
	return _neighbours.insert(neighbour);
}

bool Triangle::specifyInteriorEdge(const Edge& edge)
{
	auto edgeIt = _boundaryEdges.find(edge);
	if (edgeIt == _boundaryEdges.end())
	{// Try in different order of nodes.
		edgeIt = _boundaryEdges.find(edge.inverted());
		if (edgeIt == _boundaryEdges.end())
		{
			std::cout << " ### " << __FUNCTION__ << " ERROR: No such edge in boundaries anymore: [" << edge.first << "," << edge.second << "]" << std::endl;
			std::cout << " ### " << __FUNCTION__ << " Triangle at: " << _firstIndexPos << ", vertex indices: [" << _vertexIndices[_firstIndexPos] << ","
					  << _vertexIndices[_firstIndexPos +1] << "," << _vertexIndices[_firstIndexPos +2] << "]" << std::endl;

			return false;
		}
	}
	assert(edgeIt != _boundaryEdges.end());

	// If this in an interior edge, it does not belong to a TriangleIsland boundary.
	_boundaryEdges.erase(edgeIt);
	return true;
}

std::set<Edge> Triangle::composeEdges()
{
	const std::vector<uint32_t> indices = getVertexIndices();
	const uint32_t i0 = indices[0];
	const uint32_t i1 = indices[1];
	const uint32_t i2 = indices[2];

	std::set<Edge> edges;
	edges.insert({i0,i1});
	edges.insert({i1,i2});
	edges.insert({i2,i0});
	return edges;
}

TriangleIsland::TriangleIsland(TriangleShared& initialTriangle, uint32_t& trianglesLeft)
{
	Chronograph chronograph(__FUNCTION__, true);

	TrianglesSet toAdd(compareTriangles);
	toAdd.insert(initialTriangle);
	Triangles withNeighboursToAdd;

	while(!toAdd.empty())
	{
		for(TriangleShared triangle: toAdd)
		{// We only add triangles to island here!
			assert(trianglesLeft > 0);
			assert(!triangle->isAdded());
			triangle->setAdded();
			--trianglesLeft;
			_triangles.push_back(triangle);
			withNeighboursToAdd.push_back(triangle);
		}

		toAdd.clear();

		for(TriangleShared triangle: withNeighboursToAdd)
		{// Here we collect neighbours to be added.
			for(TriangleShared neighbourTriangle: triangle->getNeighbours())
			{// For each neighbour of this triangle, check if it was already added...
				if (neighbourTriangle->isAdded())
					continue;
				// ... and in case not, collect in in a set of triangles to add.
				toAdd.insert(neighbourTriangle);
			}
		}

		withNeighboursToAdd.clear();
	}

	// Collect edges from all island triangles.
	for(const TriangleShared& triangle : _triangles)
	{
		for (const Edge& edge : triangle->getBoundaryEdges())
		{
			_boundaryEdges.insert(edge);
		}
	}
}

Triangles& TriangleIsland::getTriangles()
{
	return _triangles;
}

const Triangles& TriangleIsland::getTriangles() const
{
	return _triangles;
}

const std::set<Edge>& TriangleIsland::getBoundaryEdges() const
{
	return _boundaryEdges;
}

std::vector<uint32_t> TriangleIsland::getTriangleIndices() const
{
	std::vector<uint32_t> islandTriangleIndices;
	islandTriangleIndices.reserve(3*getTriangles().size());
	for (TriangleShared triangle : getTriangles())
	{
		islandTriangleIndices.push_back(triangle->getVertexIndex(0));
		islandTriangleIndices.push_back(triangle->getVertexIndex(1));
		islandTriangleIndices.push_back(triangle->getVertexIndex(2));
	}
	return islandTriangleIndices;
}
