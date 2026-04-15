#pragma once

#include <string>
#include <unordered_map>
#include <vector>


namespace task2 {

struct Node {
	static constexpr float DefaultWidth = 180.0f;
	static constexpr float DefaultHeight = 80.0f;
	
	int id{};
	std::string name;
	float x{ 0.0f };
	float y{ 0.0f };
	float width{ DefaultWidth };
	float height{ DefaultHeight };
};

struct Edge {
	int from{};
	int to{};
};

class Graph {
public:
	bool addNode( Node node );
	bool addEdge( Edge edge );
	void clear();

	Node* findNode( int id );
	const Node* findNode( int id ) const;

	bool hasNode( int id ) const;

	const std::unordered_map< int, Node >& getNodes() const { return nodes_; }

	const std::vector< Edge >& getEdges() const { return edges_; }

private:
	std::unordered_map< int, Node > nodes_;
	std::vector< Edge > edges_;
};

}  // namespace task2