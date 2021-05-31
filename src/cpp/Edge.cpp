#include "Edge.h"

#include <cassert>
#include <set>

Edge::Edge(const Edge& other)
{
	*this = other;
}

Edge::Edge(uint32_t f, uint32_t s)
{
	first = f;
	second = s;
}

Edge::Edge(const std::pair<uint32_t, uint32_t>& pair)
{
	_endpoints = pair;
}

Edge::Edge(const std::initializer_list<uint32_t>& list)
{
	assert(2 == list.size());
	first = *list.begin();
	second = *std::rbegin(list);
}

Edge& Edge::operator=(const Edge& other)
{
	_endpoints = other._endpoints;
	return *this;
}

bool Edge::operator==(const Edge& other) const
{
	return _endpoints == other._endpoints;
}

bool Edge::operator!=(const Edge& other) const
{
	return _endpoints != other._endpoints;
}

bool Edge::operator<(const Edge& other) const
{
	return _endpoints < other._endpoints;
}
