#include "layout_engine.hpp"

#include <algorithm>
#include <queue>

namespace task2 {

namespace {
void resolveCycles( const Graph& graph, std::unordered_map< int, int >& layer ) {
	const auto& nodes = graph.getNodes();
	const auto& edges = graph.getEdges();

	for ( int i = 0; i < static_cast< int >( nodes.size() ); ++i ) {
		bool changed = false;
		for ( const auto& edge : edges ) {
			if ( layer[ edge.to ] < layer[ edge.from ] + 1 ) {
				layer[ edge.to ] = layer[ edge.from ] + 1;
				changed          = true;
			}
		}
		if ( !changed ) break;
	}
}

std::unordered_map< int, int > computeLayersInternal( const Graph& graph,
                                                      const std::unordered_map< int, std::vector< int > >& outgoing ) {
	std::unordered_map< int, int > layer;
	std::unordered_map< int, int > indegree;
	const auto& nodes = graph.getNodes();

	for ( const auto& [ id, _ ] : nodes ) {
		layer[ id ]    = 0;
		indegree[ id ] = 0;
	}

	for ( const auto& edge : graph.getEdges() ) {
		indegree[ edge.to ]++;
	}

	std::queue< int > q;
	for ( const auto& [ id, _ ] : nodes ) {
		if ( indegree[ id ] == 0 ) q.push( id );
	}

	if ( q.empty() ) {
		for ( const auto& [ id, _ ] : nodes ) q.push( id );
	}

	size_t visited = 0;
	while ( !q.empty() ) {
		int curr = q.front();
		q.pop();
		visited++;

		if ( outgoing.contains( curr ) ) {
			for ( int next : outgoing.at( curr ) ) {
				layer[ next ] = std::max( layer[ next ], layer[ curr ] + 1 );
				if ( --indegree[ next ] == 0 ) q.push( next );
			}
		}
	}

	if ( visited < nodes.size() ) {
		resolveCycles( graph, layer );
	}

	return layer;
}
}  // namespace

LayoutEngine::LayoutEngine() : config_() {}

LayoutEngine::LayoutEngine( const Config& config ) : config_( config ) {}

void LayoutEngine::applyLayout( Graph& graph ) const {
	const auto& nodes = graph.getNodes();
	if ( nodes.empty() ) return;

	std::unordered_map< int, std::vector< int > > outgoing;
	std::unordered_map< int, std::vector< int > > incoming;
	for ( const auto& [ id, node ] : nodes ) {
		outgoing[ id ] = {};
		incoming[ id ] = {};
	}
	for ( const auto& edge : graph.getEdges() ) {
		outgoing[ edge.from ].push_back( edge.to );
		incoming[ edge.to ].push_back( edge.from );
	}

	auto layerMap = computeLayersInternal( graph, outgoing );

	arrangeNodesInLayers( graph, layerMap, incoming );
}

void LayoutEngine::arrangeNodesInLayers( Graph& graph, const LayerMap& layer, const AdjList& incoming ) const {
	std::unordered_map< int, std::vector< Node* > > layers;
	for ( const auto& [ id, _ ] : graph.getNodes() ) {
		if ( Node* ptr = graph.findNode( id ) ) {
			layers[ layer.at( id ) ].push_back( ptr );
		}
	}

	for ( auto& [ layer_idx, nodes_in_layer ] : layers ) {
		std::ranges::sort( nodes_in_layer, [ &incoming ]( const Node* a, const Node* b ) {
			if ( incoming.at( a->id ).size() != incoming.at( b->id ).size() ) {
				return incoming.at( a->id ).size() < incoming.at( b->id ).size();
			}
			return a->id < b->id;
		} );

		const float current_x = config_.margin_x + ( static_cast< float >( layer_idx ) * config_.layer_spacing );
		for ( size_t i = 0; i < nodes_in_layer.size(); ++i ) {
			nodes_in_layer[ i ]->x = current_x;
			nodes_in_layer[ i ]->y = config_.margin_y + ( static_cast< float >( i ) * config_.node_spacing );
		}
	}
}

}  // namespace task2