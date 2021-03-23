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
	setupNeighbours();
}

std::vector<IslandShared> TriangleConnectivity::operator()()
{
	return calculateIslands();
}

void TriangleConnectivity::createTriangles()
{
	for (uint32_t i : _indices)
	{
		_triangles.push_back(std::make_shared<Triangle>(i, _indices));
	}
}

void TriangleConnectivity::setupNeighbours()
{
	std::map<uint32_t, std::set<uint32_t>> vertex_to_tris;

	const uint32_t numIndices = uint32_t(_indices.size());
	for (uint32_t i=0; i<numIndices; i+=3)
	{
		vertex_to_tris[_indices[i]].insert(i);
		vertex_to_tris[_indices[i+1]].insert(i);
		vertex_to_tris[_indices[i+2]].insert(i);
	}

	const uint32_t numTriangles = uint32_t(_triangles.size());
	for (uint32_t i = 0; i < numTriangles; ++i)
	{
		Triangle& t = *_triangles[i];
		std::set<uint32_t> temp_neighbours;
		for (uint32_t j = 0; j < 3; ++j)
		{
			uint32_t test_index = uint32_t(t.getIndex(j));
			for (std::set<uint32_t>::iterator it = vertex_to_tris[test_index].begin(); it != vertex_to_tris[test_index].end(); ++it)
			{
				if (*it != i) temp_neighbours.insert(*it/3); //divide by 3 to get the 'actual' tri index
			}
		}

		for (std::set<uint32_t>::iterator it = temp_neighbours.begin(); it != temp_neighbours.end(); ++it)
		{
			Triangle& other = *_triangles[*it];
			t.addNeighbour(other);
		}
	}
}

std::vector<std::shared_ptr<Island>> TriangleConnectivity::calculateIslands()
{
	std::vector<std::shared_ptr<Island>> islandsRet;
	for (uint32_t i=0; i<uint32_t(_triangles.size()); ++i)
	{
		Triangle& t = *_triangles[i];
		if(!t.isAdded())
		{
			islandsRet.push_back(std::make_shared<Island>());
			islandsRet.back()->recursiveAdd(t);
		}
	}
	return islandsRet;
}

uint32_t Triangle::getIndex(uint32_t i) const
{
	return _indices[_positionIndex + i];
}

Triangle::Triangle(uint32_t positionIndex, const std::vector<uint32_t>& indices) :
	_positionIndex(positionIndex),
	_indices(indices),
	_isAdded(false)
{

}

bool Triangle::isNeighbour(const Triangle& other)
{
	for (uint32_t i=0; i<3; ++i)
	{
		for (uint32_t j=0; j<3; ++j)
		{
			if (getIndex(i) != other.getIndex(j))
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

Triangle& Triangle::getNeighbour(uint32_t i)
{
	return *_neighbours[i];
}

void Triangle::addNeighbour(Triangle& neighbour)
{
	_neighbours.push_back(std::shared_ptr<Triangle>(&neighbour));//changed to set
}

void Island::recursiveAdd(Triangle& t)
{
	addAndSetAdded(t);
	for(uint32_t i = 0; i < t.getNeighbourCount(); i++)
	{
		if (t.getNeighbour(i).isAdded())
			continue;

		recursiveAdd(t.getNeighbour(i));
	}
}

std::set<TriangleShared> Island::get()
{
	return _triangles;
}
void Island::addAndSetAdded(Triangle& t)
{
	t.setAdded();
	_triangles.insert(TriangleShared(&t));
}
