#pragma once

#include <string>
#include <vector>

namespace task2 {

struct Node {
	int id{};
	std::string name;
	float x{ 0.0f };
	float y{ 0.0f };
	float width{ 180.0f };
	float height{ 80.0f };
};

struct Edge {
	int from{};
	int to{};
};

class Graph {
public:
	std::vector< Node > nodes;
	std::vector< Edge > edges;

	Node* findNode( int id );
	const Node* findNode( int id ) const;

	bool hasNode( int id ) const;
	bool hasUniqueNodeIds() const;
};

}  // namespace task2