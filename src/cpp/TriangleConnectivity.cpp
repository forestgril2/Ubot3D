#include "TriangleConnectivity.h"
#include <iostream>     // std::cout
#include <algorithm>    // std::set_difference, std::sort
#include <vector>       // std::vector
#include <set>       // std::vector
#include <cmath>
#include <map>
#include <memory>

#include <Chronograph.h>

#undef NDEBUG

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

void TriangleConnectivity::composeTriangleNeighbourhoodAtVertex(TriangleShared thisTriangle, TriangleShared neighbour, uint32_t vertexIndex,
														std::map<TriangleShared, std::pair<Edge, bool>>& edgesAtNewNeighbours)
{
	const bool isNewNeighbour = thisTriangle->addNeighbour(neighbour).second;
	if (isNewNeighbour)
	{// Also add this one to the other triangle as neighbour.
		#ifndef NDEBUG
		const bool isAddedToTheOtherForTheFirstTime =
		#endif
		neighbour->addNeighbour(thisTriangle).second;
		assert(isAddedToTheOtherForTheFirstTime);

		// Remember the vertex index, at whcih this neighbour was found.
		#ifndef NDEBUG
		auto [neighbourThisRoundIt, isNewInThisTriangleRound] =
		#endif

		// We need a second addition of this neighbour in this triangle round to resolve this neighbourhood as a common edge.
		edgesAtNewNeighbours.insert({neighbour, {{vertexIndex, kMaxUnsigned}, /*isValid*/ false}});
		return;
	}

	auto neighbourIt = edgesAtNewNeighbours.find(neighbour);
	const bool neighbourExistsAtVertexInThisRound = (neighbourIt != edgesAtNewNeighbours.end());

	if (!neighbourExistsAtVertexInThisRound)
		return; // This means that the neighbourhood has already been resolved at the other "neighbour" triangle

	// If a vertex is already mapped to the "neighbour" for the second time in this triangle round,
	// report common edge resolution, by filling fill a valid common edge data.
	const uint32_t previousIndex = neighbourIt->second.first.first;
	neighbourIt->second = {std::minmax(previousIndex, vertexIndex), true};
}

void TriangleConnectivity::setupTriangleNeighboursAndEdges()
{
	Chronograph chronograph(__FUNCTION__, true);

	// Every triangle has 3 vertices, but the same vertex may be common to many triangles.
	const std::map<uint32_t, std::set<TriangleShared>> trianglesAtVertex = getTrianglesAtVertices();

	for (TriangleShared triangle : _triangles)
	{// For every triangle
		// Remember, which new triangles were added in this triangle round. Map to them vertices/edges, at which they were added.
		std::map<TriangleShared, std::pair<Edge, bool>> edgesToNewNeighbours;

		for (const uint32_t vertexIndex : triangle->getVertexIndices())
		{// For every vertex in triangle.
			for (TriangleShared neighbour : trianglesAtVertex.at(vertexIndex))
			{// For every neighbour triangle incident to this vertex.
				if (neighbour == triangle)
					continue; // Skip the triangle in question.

				// Setup neighbourhood of this triangle and the neighbour at this vertex.
				composeTriangleNeighbourhoodAtVertex(triangle, neighbour, vertexIndex, edgesToNewNeighbours);

				const auto edgeAtNeighbourIt = edgesToNewNeighbours.find(neighbour);
				if (edgeAtNeighbourIt == edgesToNewNeighbours.end())
					continue; // Neighbour is already resolved at the other triangle

				if (/*isEdgeValid*/ !edgeAtNeighbourIt->second.second)
					continue; // Edge is invalid.

				// The two triangles share a common interior edge - one, which is not an island boundary.
				const Edge& validCommonEdge = edgeAtNeighbourIt->second.first;
				triangle->specifyInteriorEdge(validCommonEdge);
				neighbour->specifyInteriorEdge(validCommonEdge);
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

std::set<uint32_t> Triangle::getVertexIndices() const
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
	_edges(calculateEdges()),
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

void Triangle::specifyInteriorEdge(const std::pair<uint32_t, uint32_t>& edge)
{
	_boundaryEdges.erase(_boundaryEdges.find(edge));
}

std::set<Edge> Triangle::calculateEdges()
{
	const std::set<uint32_t> indices = getVertexIndices();
	auto indicesIt = indices.begin();
	const uint32_t i0 = *indicesIt;
	++indicesIt;
	const uint32_t i1 = *indicesIt;
	++indicesIt;
	const uint32_t i2 = *indicesIt;

	std::set<Edge> edges;
	edges.insert({i0,i1});
	edges.insert({i0,i2});
	edges.insert({i1,i2});
	return edges;
}

TriangleIsland::TriangleIsland(TriangleShared& initialTriangle, uint32_t& trianglesLeft)
{
	Chronograph chronograph(__FUNCTION__, true);
	static uint32_t counter = 0;
	_myNumber = counter++;	

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

	calculateBoundaries();
}

Triangles& TriangleIsland::getTriangles()
{
	return _triangles;
}

const Triangles& TriangleIsland::getTriangles() const
{
	return _triangles;
}

const std::set<Edge>& TriangleIsland::getEdges() const
{
	return _edges;
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

void TriangleIsland::calculateBoundaries()
{
	Chronograph chronograph(__FUNCTION__, true);

	// The key in this map are boundary limiting points. If they match, the boundary is closed, with all edges connected.
	std::vector<std::list<uint32_t>> boundaries;
	std::set<Edge> edgesLeft;

	// We will also use a mapping of all free edges to their vertices;
	std::map<uint32_t, std::set<Edge>> edgesToVertex;

	// Initialize the maps with separate edges.
	for(const TriangleShared &triangle : _triangles)
	{
		for (const auto &edge : triangle->getBoundaryEdges())
		{
			edgesLeft.insert(edge);
			edgesToVertex[edge.first].insert(edge);
			edgesToVertex[edge.second].insert(edge);
		}
	}

	for (const auto& [vertex, edges] : edgesToVertex)
	{// All these sets should start wita a size of 2n.
		if (0 != edges.size() % 2)
		{
			std::cout << " ### " << __FUNCTION__ << " ERROR! Impossible thing has happened: there are " << edges.size() << " edges at one boundary node." << std::endl;
			exit(-1);
		}
		assert(0 == edges.size() % 2);
	}

	_edges = edgesLeft;

//	auto edgesIt = edgesLeft.begin();
//	while(edgesIt != edgesLeft.end())
//	{// Initialize a boundary with an edge and keep finding adjacent edges, adding their vertices, until the boundary is closed.
//		std::list<uint32_t> boundary{edgesIt->first, edgesIt->second};
//		edgesLeft.erase(*edgesIt);

//		Edge edgeFront = std::minmax(boundary.front(), boundary.back());
//		Edge edgeBack = edgeFront;

//		auto [vertexFront, vertexBack] = edgeFront;

//		while (vertexFront != vertexBack)
//		{
//			auto& edgesAtVertexFront = edgesToVertex.at(vertexFront);
//			auto& edgesAtVertexBack = edgesToVertex.at(vertexBack);

//			edgesAtVertexFront.erase(edgeFront);
//			edgesAtVertexBack.erase(edgeBack);

//			edgeFront = *edgesAtVertexFront.begin();
//			edgeBack = *edgesAtVertexBack.begin();

//			edgesLeft.erase(edgeFront);
//			edgesLeft.erase(edgeBack);

//			// Now edge sets at limiting vertices are left with only one edge per set - the exterior/not connected ones.

////			boundary.
//		}

//		++edgesIt;
//	}
}
