#include "graph.hpp"

#include <unordered_set>

namespace task2 {

Node* Graph::findNode( int id ) {
	for ( auto& node : nodes ) {
		if ( node.id == id ) {
			return &node;
		}
	}
	return nullptr;
}

const Node* Graph::findNode( int id ) const {
	for ( const auto& node : nodes ) {
		if ( node.id == id ) {
			return &node;
		}
	}
	return nullptr;
}

bool Graph::hasNode( int id ) const {
	return findNode( id ) != nullptr;
}

bool Graph::hasUniqueNodeIds() const {
	std::unordered_set< int > ids;
	for ( const auto& node : nodes ) {
		if ( !ids.insert( node.id ).second ) {
			return false;
		}
	}
	return true;
}

}  // namespace task2