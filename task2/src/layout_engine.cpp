#include "layout_engine.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>

namespace task2 {

LayoutEngine::LayoutEngine() : config_() {}

LayoutEngine::LayoutEngine( const Config& config ) : config_( config ) {}

void LayoutEngine::applyLayout( Graph& graph ) const {
	if ( graph.nodes.empty() ) {
		return;
	}

	std::unordered_map< int, std::vector< int > > outgoing;
	std::unordered_map< int, std::vector< int > > incoming;
	std::unordered_map< int, int > indegree;
	std::unordered_map< int, int > layer;

	for ( const auto& node : graph.nodes ) {
		outgoing[ node.id ] = {};
		incoming[ node.id ] = {};
		indegree[ node.id ] = 0;
		layer[ node.id ]    = 0;
	}

	for ( const auto& edge : graph.edges ) {
		outgoing[ edge.from ].push_back( edge.to );
		incoming[ edge.to ].push_back( edge.from );
		indegree[ edge.to ]++;
	}

	std::queue< int > q;
	for ( const auto& node : graph.nodes ) {
		if ( indegree[ node.id ] == 0 ) {
			q.push( node.id );
		}
	}

	if ( q.empty() ) {
		for ( const auto& node : graph.nodes ) {
			q.push( node.id );
		}
	}

	int visited_count = 0;

	while ( !q.empty() ) {
		const int current = q.front();
		q.pop();
		visited_count++;

		for ( const int next : outgoing[ current ] ) {
			layer[ next ] = std::max( layer[ next ], layer[ current ] + 1 );
			indegree[ next ]--;

			if ( indegree[ next ] == 0 ) {
				q.push( next );
			}
		}
	}

	if ( visited_count < static_cast< int >( graph.nodes.size() ) ) {
		for ( const auto& node : graph.nodes ) {
			if ( !layer.contains( node.id ) ) {
				layer[ node.id ] = 0;
			}
		}

		bool changed = true;
		for ( int iteration = 0; iteration < static_cast< int >( graph.nodes.size() ) && changed; ++iteration ) {
			changed = false;
			for ( const auto& edge : graph.edges ) {
				if ( layer[ edge.to ] < layer[ edge.from ] + 1 ) {
					layer[ edge.to ] = layer[ edge.from ] + 1;
					changed          = true;
				}
			}
		}
	}

	std::unordered_map< int, std::vector< Node* > > layers;
	for ( auto& node : graph.nodes ) {
		layers[ layer[ node.id ] ].push_back( &node );
	}

	for ( auto& [ layer_index, nodes_in_layer ] : layers ) {
		std::ranges::sort( nodes_in_layer, [ &incoming ]( const Node* lhs, const Node* rhs ) {
			const auto lhs_in = incoming[ lhs->id ].size();
			const auto rhs_in = incoming[ rhs->id ].size();
			if ( lhs_in != rhs_in ) {
				return lhs_in < rhs_in;
			}
			return lhs->id < rhs->id;
		} );

		for ( std::size_t i = 0; i < nodes_in_layer.size(); ++i ) {
			Node* node = nodes_in_layer[ i ];
			node->x    = config_.margin_x + ( static_cast< float >( layer_index ) * config_.layer_spacing );
			node->y    = config_.margin_y + ( static_cast< float >( i ) * config_.node_spacing );
		}
	}
}

}  // namespace task2