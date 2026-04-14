#include "graph.hpp"

namespace task2 {

bool Graph::addNode( Node node ) {
	auto result = nodes_.insert( { node.id, std::move( node ) } );
	return result.second;
}

bool Graph::addEdge( Edge edge ) {
	if ( !hasNode( edge.from ) || !hasNode( edge.to ) ) {
		return false;
	}
	edges_.push_back( edge );
	return true;
}

void Graph::clear() {
	nodes_.clear();
	edges_.clear();
}

Node* Graph::findNode( int id ) {
	auto it = nodes_.find( id );
	return ( it != nodes_.end() ) ? &it->second : nullptr;
}

const Node* Graph::findNode( int id ) const {
	auto it = nodes_.find( id );
	return ( it != nodes_.end() ) ? &it->second : nullptr;
}

bool Graph::hasNode( int id ) const {
	return nodes_.contains( id );
}

}  // namespace task2