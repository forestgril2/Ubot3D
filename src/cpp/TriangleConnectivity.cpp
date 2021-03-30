#include "TriangleConnectivity.h"
#include <iostream>     // std::cout
#include <algorithm>    // std::set_difference, std::sort
#include <vector>       // std::vector
#include <set>       // std::vector
#include <cmath>
#include <map>
#include <memory>


static bool compareTriangles(const TriangleWeak& t1, const TriangleWeak& t2)
{
	return t1.lock()->getVertexIndex(0) < t2.lock()->getVertexIndex(0);
}

TriangleConnectivity::TriangleConnectivity(const std::vector<uint32_t>& indices) :
	_indices(indices)
{
	createTriangles();
	setupTriangleNeighbours();
}

void TriangleConnectivity::createTriangles()
{
	for (uint32_t firstTriangleIndexPos=0; firstTriangleIndexPos<_indices.size(); firstTriangleIndexPos += 3)
	{// Jump through all first triangle vertex indices.
		_triangles.push_back(std::make_shared<Triangle>(firstTriangleIndexPos, _indices));
	}
}

void TriangleConnectivity::setupTriangleNeighbours()
{
	// For every vertex index, assign triangle indices,
	// for triangles incident to that vertex.
	std::map<uint32_t, std::set<uint32_t>> trianglesToVertex;
	const uint32_t numIndices = uint32_t(_indices.size());
	assert(0 == numIndices % 3);
	for (uint32_t trianglePos=0; trianglePos<numIndices; trianglePos+=3)
	{
		auto insertion = trianglesToVertex[_indices[trianglePos   ]].insert(trianglePos/3);
		assert(insertion.second == true);
		insertion = trianglesToVertex[_indices[trianglePos +1]].insert(trianglePos/3);
		assert(insertion.second == true);
		insertion = trianglesToVertex[_indices[trianglePos +2]].insert(trianglePos/3);
		assert(insertion.second == true);
	}

	const uint32_t numTriangles = uint32_t(_triangles.size());
	for (uint32_t triangleIndex=0; triangleIndex<numTriangles; ++triangleIndex)
	{// For every triangle.
		Triangle& triangle = *_triangles[triangleIndex];

		std::set<uint32_t> foundTriangleNeighbours;
		for (uint32_t v=0; v<3; ++v)
		{// For every vertex in triangle.
			for (uint32_t incidentTriangleIndex : trianglesToVertex[triangle.getVertexIndex(v)])
			{// For every triangle incident to this vertex.
				if (incidentTriangleIndex == triangleIndex)
					continue; // Skip index for the triangle in question.
				triangle.addNeighbour(_triangles[incidentTriangleIndex]);
			}
		}
	}
}

std::vector<TriangleIsland> TriangleConnectivity::calculateIslands()
{
	std::vector<TriangleIsland> islands;
	for (TriangleShared& triangle : _triangles)
	{
		if(triangle->isAdded())
			continue;
		islands.push_back(TriangleIsland());
		islands.back().recursiveAdd(triangle);
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

bool Triangle::isAdded() const
{
	return _isAdded;
}

void Triangle::setAdded()
{
	_isAdded = true;
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
	_neighbours.insert(TriangleWeak(neighbour));
}

void TriangleIsland::recursiveAdd(TriangleShared& triangle)
{
	addAndSetAdded(triangle);
	for(TriangleWeak neighbourTriangle: triangle->getNeighbours())
	{
		TriangleShared neighbour = neighbourTriangle.lock();
		assert(neighbour);
		if (neighbour->isAdded())
			continue;

		recursiveAdd(neighbour);
	}
}

TriangleSet& TriangleIsland::getTriangles()
{
	return _triangles;
}
void TriangleIsland::addAndSetAdded(TriangleShared& triangle)
{
	triangle->setAdded();
	_triangles.insert(triangle);
}
