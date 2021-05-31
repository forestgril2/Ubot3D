#pragma once

#include <iostream>
#include <stdint.h>
#include <utility>

class Edge
{
public:
	Edge(const Edge& other);
	Edge(uint32_t f, uint32_t s);
	Edge(const std::pair<uint32_t, uint32_t>& pair);
	Edge(const std::initializer_list<uint32_t>& list);

	Edge& operator=(const Edge& other);
	bool operator==(const Edge& other) const;
	bool operator!=(const Edge& other) const;
	bool operator<(const Edge& other) const;

	uint32_t& first = _endpoints.first;
	uint32_t& second = _endpoints.second;

	friend std::ostream& operator<<(std::ostream& stream, const Edge& edge)
	{
		stream << "Edge[" << edge.first << "," << edge.second << "]";
		return stream;
	}

private:
	std::pair<uint32_t, uint32_t> _endpoints;
};
