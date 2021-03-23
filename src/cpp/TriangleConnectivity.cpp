#include "TriangleConnectivity.h"
#include <iostream>     // std::cout
#include <algorithm>    // std::set_difference, std::sort
#include <vector>       // std::vector
#include <set>       // std::vector
#include <cmath>
#include <map>
#include <memory>

TriangleConnectivity::TriangleConnectivity(const std::vector<uint32_t>& indices) :
	_indices(indices)
{
	createTriangles();
	setupTriangleNeighbours();
}

std::vector<IslandShared> TriangleConnectivity::operator()()
{
	return calculateIslands();
}

void TriangleConnectivity::createTriangles()
{
	for (uint32_t index=0; index<_indices.size(); index += 3)
	{// Jump through all first triangle vertex indices.
		_triangles.push_back(std::make_shared<Triangle>(index, _indices));
	}
}

void TriangleConnectivity::setupTriangleNeighbours()
{
	// For every vertex index, assign a set of indices,
	// which mark (first vertices of) triangles incident to that vertex.
	std::map<uint32_t, std::set<uint32_t>> trianglesToVertex;

	const uint32_t numIndices = uint32_t(_indices.size());
	for (uint32_t vertexIndex=0; vertexIndex<numIndices; vertexIndex+=3)
	{
		std::pair<std::set<uint32_t>::iterator, bool> insertion;
		insertion = trianglesToVertex[_indices[vertexIndex   ]].insert(vertexIndex);
		assert(insertion.second == true);
		insertion = trianglesToVertex[_indices[vertexIndex +1]].insert(vertexIndex);
		assert(insertion.second == true);
		insertion = trianglesToVertex[_indices[vertexIndex +2]].insert(vertexIndex);
		assert(insertion.second == true);
	}

	const uint32_t numTriangles = uint32_t(_triangles.size());
	for (uint32_t triangleIndex=0; triangleIndex<numTriangles; ++triangleIndex)
	{// For every triangle.
		Triangle& triangle = *_triangles[triangleIndex];

		std::set<uint32_t> foundTriangleNeighbours;
		for (uint32_t v=0; v<3; ++v)
		{// For every vertex in triangle.
			uint32_t vertexIndexInTriangle = uint32_t(triangle.getVertexIndex(v));
			for (uint32_t incidentTriangleFirstIndex : trianglesToVertex[vertexIndexInTriangle])
			{// For every triangle incident to this vertex.
				if (incidentTriangleFirstIndex == triangleIndex)
					continue; // Skip index for the triangle in question.
				// Divide by 3 to get the 'actual' neighbour triangle index.
				foundTriangleNeighbours.insert(incidentTriangleFirstIndex/3);
			}
		}

		for (uint32_t neighbourTriangleIndex : foundTriangleNeighbours)
		{
			Triangle& other = *_triangles[neighbourTriangleIndex];
			triangle.addNeighbour(other);
		}
	}
}

std::vector<std::shared_ptr<TriangleIsland>> TriangleConnectivity::calculateIslands()
{
	std::vector<std::shared_ptr<TriangleIsland>> islands;
	for (TriangleShared& triangle : _triangles)
	{
		Triangle& t = *triangle;
		if(t.isAdded())
			continue;
		islands.push_back(std::make_shared<TriangleIsland>());
		islands.back()->recursiveAdd(t);
	}
	return islands;
}

uint32_t Triangle::getVertexIndex(uint32_t i) const
{
	return _vertexIndices[_firstIndexPos + i];
}

Triangle::Triangle(uint32_t index, const std::vector<uint32_t>& indices) :
	_firstIndexPos(index),
	_vertexIndices(indices),
	_isAdded(false)
{
}

bool Triangle::isNeighbour(const Triangle& other)
{
	for (uint32_t i=0; i<3; ++i)
	{
		for (uint32_t j=0; j<3; ++j)
		{
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

Triangles& Triangle::getNeighbours()
{
	return _neighbours;
}

void Triangle::addNeighbour(Triangle& neighbour)
{
	_neighbours.push_back(std::shared_ptr<Triangle>(&neighbour));//changed to set
}

void TriangleIsland::recursiveAdd(Triangle& triangle)
{
	addAndSetAdded(triangle);
	for(TriangleShared& neighbourTriangle: triangle.getNeighbours())
	{
		if (neighbourTriangle->isAdded())
			continue;

		recursiveAdd(*neighbourTriangle);
	}
}

std::set<TriangleShared> TriangleIsland::getTriangles()
{
	return _triangles;
}
void TriangleIsland::addAndSetAdded(Triangle& t)
{
	t.setAdded();
	_triangles.insert(TriangleShared(&t));
}
