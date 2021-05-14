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

static bool compareTriangles(const TriangleShared& t1, const TriangleShared& t2)
{
	return *t1 < *t2;
}

TriangleConnectivity::TriangleConnectivity(const std::vector<uint32_t>& indices) :
	_indices(indices)
{
	Chronograph chronograph(__FUNCTION__, false);
	assert(0 == _indices.size() % 3);
	createTriangles();
	setupTriangleNeighboursAndEdges();
}

void TriangleConnectivity::createTriangles()
{
	Chronograph chronograph(__FUNCTION__, false);
	_triangles.reserve(_indices.size()/3);
	for (uint32_t firstTriangleIndexPos=0; firstTriangleIndexPos<_indices.size(); firstTriangleIndexPos += 3)
	{// Jump through all first triangle vertex indices.
		_triangles.push_back(std::make_shared<Triangle>(firstTriangleIndexPos, _indices));
//		std::cout << " ### " << __FUNCTION__ << " firstTriangleIndexPos:" << " " << firstTriangleIndexPos << "" << std::endl;
	}
}

void TriangleConnectivity::setupTriangleNeighboursAndEdges()
{
	Chronograph chronograph(__FUNCTION__, false);
	// Every triangle has 3 vertices, but many triangles may share the same vertex
	// and its corresponging index.

	// For every vertex index, assign array indices from triangle array
	// std::vector<TriangleShared> for triangles incident to that vertex.
	// TODO: Consider changing it to a pair of vectors for speed - one would
	// be a vector of unique indices (precalculated before) the other would
	// be this map, but changed to std::vector<std::vector<uint32_t>>.
	std::map<uint32_t, std::vector<uint32_t>> trianglesAtVertex;
	const uint32_t numIndices = uint32_t(_indices.size());
	for (uint32_t i=0; i<numIndices; ++i)
	{
		trianglesAtVertex[i].reserve(kNumTrianglesReservedPerVertex);
	}
	const uint32_t numTriangles = uint32_t(_triangles.size());
	for (uint32_t triangleIndex=0; triangleIndex<numTriangles; ++triangleIndex)
	{// For every triangle, push its index to collections incident to all vertices of this triangle.
		// Note: triangle index multiplied by 3, gives position of its first vertex index in _indices.
		trianglesAtVertex[_indices[triangleIndex*3   ]].push_back(triangleIndex);
		trianglesAtVertex[_indices[triangleIndex*3 +1]].push_back(triangleIndex);
		trianglesAtVertex[_indices[triangleIndex*3 +2]].push_back(triangleIndex);
	}

	for (uint32_t triangleIndex=0; triangleIndex<numTriangles; ++triangleIndex)
	{// For every triangle.
		Triangle& thisTriangle = *_triangles[triangleIndex];
		std::map<TriangleShared, uint32_t> trianglesAddedStartingFromThisTriangle;

		for (uint32_t v=0; v<3; ++v)
		{// For every vertex in triangle.
			const uint32_t thisTriangleCurrentVertexIndex = thisTriangle.getVertexIndex(v);
			for (uint32_t incidentTriangleIndex : trianglesAtVertex[thisTriangleCurrentVertexIndex])
			{// For every triangle incident to this vertex.
				if (incidentTriangleIndex == triangleIndex)
					continue; // Skip index for the triangle in question.

				// Attempt to add a neighbour triangle.
				auto [otherTriangleIterator, isAddedForTheFirstTime] = thisTriangle.addNeighbour(_triangles[incidentTriangleIndex]);
				auto [triangleAddedStartingFromThisTriangle, isAddedForTheFirstTimeStartingFromThisTriangle] =
						trianglesAddedStartingFromThisTriangle.insert({_triangles[triangleIndex], thisTriangleCurrentVertexIndex});
				if (isAddedForTheFirstTime)
				{// Also add this one to the other triangle as neighbour.
					#ifndef NDEBUG
					const auto [thisTriangleIterator, isAddedToTheOtherForTheFirstTime] =
					#endif
							_triangles[incidentTriangleIndex]->addNeighbour(_triangles[triangleIndex]);
					assert(isAddedToTheOtherForTheFirstTime);
					continue;
				}

				// If the "otherTriangle" has already been added to "thisTriangle", starting from the other,
				// we do nothing, as all has already been resolved at the other triangle.
				if (isAddedForTheFirstTimeStartingFromThisTriangle)
					continue;

				// If the "otherTriangle" has already been added to "thisTriangle", starting from "thisTriangle",
				// the two triangles share an interior edge - swhich is not an island boundary.
				TriangleShared added = *otherTriangleIterator;

				// Find out the previous edge-adjacent vertex index.
				uint32_t otherVertexIndex = std::numeric_limits<uint32_t>::max();
				if (v == 0)
				{
					std::cout << " ### " << __FUNCTION__ << " ERROR, impossible thing happened!" << std::endl;
					exit(-1);
				}
				assert(v!=0); // this should not be possible!

				if (v==1)
				{// The previous one is the vertex shared by this interior edge.
					otherVertexIndex = thisTriangle.getVertexIndex(0);
				}
				else
				{// This will be either v==0 || v==1
					// TODO : assert this one is found!
					otherVertexIndex = thisTriangle.getVertexIndex(trianglesAddedStartingFromThisTriangle.at(_triangles[triangleIndex]));
				}


				std::pair<uint32_t, uint32_t> interiorEdge{otherVertexIndex, thisTriangleCurrentVertexIndex};
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

Triangle::Triangle(uint32_t pos, const std::vector<uint32_t>& indices) :
	_neighbours(compareTriangles),
	_firstIndexPos(pos),
	_vertexIndices(indices),
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

TriangleIsland::TriangleIsland(TriangleShared& initialTriangle, uint32_t& trianglesLeft)
{
//	Chronograph chronograph(__FUNCTION__, true);
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
	for(const TriangleShared& triangle : _triangles)
	{

	}
}
