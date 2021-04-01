#include "TriangleConnectivity.h"
#include <iostream>     // std::cout
#include <algorithm>    // std::set_difference, std::sort
#include <vector>       // std::vector
#include <set>       // std::vector
#include <cmath>
#include <map>
#include <memory>

#include <Chronograph.h>

// Most well-triangulated meshes should have around 6 triangles shared by one vertex. Let's reserve for 8.
static const uint32_t kNumTrianglesReservedPerVertex = 8;

static bool compareTriangles(const TriangleWeak& t1, const TriangleWeak& t2)
{
	return *t1.lock() < *t2.lock();
}

TriangleConnectivity::TriangleConnectivity(const std::vector<uint32_t>& indices) :
	_indices(indices)
{
	Chronograph chronograph(__FUNCTION__, true);
	assert(0 == _indices.size() % 3);
	createTriangles();
	setupTriangleNeighbours();
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

void TriangleConnectivity::setupTriangleNeighbours()
{
	Chronograph chronograph(__FUNCTION__, true);
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
	{// For every triangle, push this triangle index to collections assigned to all its vertices.
		// Note: triangle index multiplied by 3, gives position of its first vertex index in _indices.
		trianglesAtVertex[_indices[triangleIndex*3   ]].push_back(triangleIndex);
		trianglesAtVertex[_indices[triangleIndex*3 +1]].push_back(triangleIndex);
		trianglesAtVertex[_indices[triangleIndex*3 +2]].push_back(triangleIndex);
	}

	for (uint32_t triangleIndex=0; triangleIndex<numTriangles; ++triangleIndex)
	{// For every triangle.
		Triangle& triangle = *_triangles[triangleIndex];

		for (uint32_t v=0; v<3; ++v)
		{// For every vertex in triangle.
			for (uint32_t incidentTriangleIndex : trianglesAtVertex[triangle.getVertexIndex(v)])
			{// For every triangle incident to this vertex.
				if (incidentTriangleIndex == triangleIndex)
					continue; // Skip index for the triangle in question.
				triangle.addNeighbour(_triangles[incidentTriangleIndex]);
			}
		}
	}
}

std::vector<TriangleIsland> TriangleConnectivity::calculateIslands(const uint32_t recursiveAddLimit)
{
	Chronograph chronograph(__FUNCTION__, true);
	std::vector<TriangleIsland> islands;
	for (TriangleShared& triangle : _triangles)
	{
		if(triangle->isAdded())
			continue;
		islands.push_back(TriangleIsland());
		islands.back().recursiveAdd(triangle, recursiveAddLimit);
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

TrianglesWeakSet& Triangle::getNeighbours()
{
	return _neighbours;
}

void Triangle::addNeighbour(TriangleShared& neighbour)
{
	if (_neighbours.insert(TriangleWeak(neighbour)).second)
	{
//		std::cout << " ### " << __FUNCTION__ << " _firstIndexPos:" << _firstIndexPos << ", neighbour._firstIndexPos: " << neighbour->_firstIndexPos << std::endl;
	}
	else
	{
//		std::cout << " ### " << __FUNCTION__ << " _firstIndexPos:" << _firstIndexPos << ", CANNOT INSERT neighbour._firstIndexPos: " << neighbour->_firstIndexPos << std::endl;
	}
}

TriangleIsland::TriangleIsland()
{
	static uint32_t counter = 0;
	_myNumber = counter++;
//	std::cout << " ### " << __FUNCTION__ << " _myNumber:" << _myNumber << "," << "" << std::endl;
}

void TriangleIsland::recursiveAdd(TriangleShared& triangle, const uint32_t recursiveAddLimit)
{
	assert(!triangle->isAdded());
//	Chronograph chronograph(__FUNCTION__, true);
	addAndSetAdded(triangle);

	static uint32_t recursionCount = 0;
	if (recursionCount > recursiveAddLimit)
		return;

	for(TriangleWeak neighbourTriangle: triangle->getNeighbours())
	{// For each neighbour of this triangle, check if it was already added...
		TriangleShared neighbour = neighbourTriangle.lock();
		assert(neighbour);
		if (neighbour->isAdded())
			continue;
		// ... and in case not, add it together with its neighbours.
		++recursionCount;
		recursiveAdd(neighbour, recursiveAddLimit);
		--recursionCount;
	}
}

TriangleSet& TriangleIsland::getTriangles()
{
	return _triangles;
}
void TriangleIsland::addAndSetAdded(TriangleShared& triangle)
{
//	Chronograph chronograph(__FUNCTION__, true);
	triangle->setAdded();
	_triangles.insert(triangle);
}
