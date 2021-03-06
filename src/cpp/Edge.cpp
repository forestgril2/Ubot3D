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
	_endNodes = pair;
}

Edge::Edge(const std::initializer_list<uint32_t>& list)
{
	assert(2 == list.size());
	first = *list.begin();
	second = *std::rbegin(list);
}

Edge& Edge::operator=(const Edge& other)
{
	_endNodes = other._endNodes;
	return *this;
}

bool Edge::operator==(const Edge& other) const
{
	return _endNodes == other._endNodes;
}

bool Edge::operator!=(const Edge& other) const
{
	return _endNodes != other._endNodes;
}

bool Edge::operator<(const Edge& other) const
{
	return _endNodes < other._endNodes;
}

Edge Edge::inverted() const
{
	return Edge(second, first);
}
