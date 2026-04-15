#include "layout_engine.hpp"

#include <algorithm>
#include <map>
#include <queue>
#include <unordered_set>

namespace task2 {

namespace {
void resolveCycles( const Graph& graph, const LayoutEngine::Component& component, std::unordered_map< int, int >& layer ) {
	const auto& edges = graph.getEdges();

	for ( int i = 0; i < static_cast< int >( component.size() ); ++i ) {
		bool changed = false;

		for ( const auto& edge : edges ) {
			if ( !layer.contains( edge.from ) || !layer.contains( edge.to ) ) {
				continue;
			}

			if ( layer[ edge.to ] < layer[ edge.from ] + 1 ) {
				layer[ edge.to ] = layer[ edge.from ] + 1;
				changed          = true;
			}
		}

		if ( !changed ) {
			break;
		}
	}
}

std::unordered_map< int, int > computeLayersInternal( const Graph& graph, const LayoutEngine::Component& component,
                                                      const std::unordered_map< int, std::vector< int > >& outgoing ) {
	std::unordered_map< int, int > layer;
	std::unordered_map< int, int > indegree;
	std::unordered_set< int > componentSet( component.begin(), component.end() );

	for ( int id : component ) {
		layer[ id ]    = 0;
		indegree[ id ] = 0;
	}

	for ( const auto& edge : graph.getEdges() ) {
		if ( componentSet.contains( edge.from ) && componentSet.contains( edge.to ) ) {
			indegree[ edge.to ]++;
		}
	}

	std::queue< int > q;
	for ( int id : component ) {
		if ( indegree[ id ] == 0 ) {
			q.push( id );
		}
	}

	if ( q.empty() ) {
		for ( int id : component ) {
			q.push( id );
		}
	}

	size_t visited = 0;
	while ( !q.empty() ) {
		const int curr = q.front();
		q.pop();
		visited++;

		if ( outgoing.contains( curr ) ) {
			for ( int next : outgoing.at( curr ) ) {
				layer[ next ] = std::max( layer[ next ], layer[ curr ] + 1 );

				if ( --indegree[ next ] == 0 ) {
					q.push( next );
				}
			}
		}
	}

	if ( visited < component.size() ) {
		resolveCycles( graph, component, layer );
	}

	return layer;
}

float computeBarycenter( const task2::Node& node, const task2::Graph& graph,
                         const std::unordered_map< int, std::vector< int > >& incoming ) {
	const auto it = incoming.find( node.id );
	if ( it == incoming.end() || it->second.empty() ) {
		return static_cast< float >( node.id );
	}

	float sum = 0.0f;
	int count = 0;

	for ( int parentId : it->second ) {
		const Node* parent = graph.findNode( parentId );
		if ( parent != nullptr ) {
			sum += parent->y;
			++count;
		}
	}

	if ( count == 0 ) {
		return node.y != 0.0f ? node.y : static_cast< float >( node.id );
	}

	return sum / static_cast< float >( count );
}

std::vector< LayoutEngine::Component > computeComponents( const Graph& graph ) {
	std::unordered_map< int, std::vector< int > > undirected;

	for ( const auto& [ id, _ ] : graph.getNodes() ) {
		undirected[ id ] = {};
	}

	for ( const auto& edge : graph.getEdges() ) {
		undirected[ edge.from ].push_back( edge.to );
		undirected[ edge.to ].push_back( edge.from );
	}

	std::unordered_set< int > visited;
	std::vector< LayoutEngine::Component > components;

	for ( const auto& [ startId, _ ] : graph.getNodes() ) {
		if ( visited.contains( startId ) ) {
			continue;
		}

		LayoutEngine::Component component;
		std::queue< int > q;
		q.push( startId );
		visited.insert( startId );

		while ( !q.empty() ) {
			const int current = q.front();
			q.pop();

			component.push_back( current );

			for ( const int next : undirected[ current ] ) {
				if ( !visited.contains( next ) ) {
					visited.insert( next );
					q.push( next );
				}
			}
		}

		components.push_back( std::move( component ) );
	}

	return components;
}
}  // namespace

LayoutEngine::LayoutEngine() : config_() {}

LayoutEngine::LayoutEngine( const Config& config ) : config_( config ) {}

void LayoutEngine::applyLayout( Graph& graph ) const {
	if ( graph.getNodes().empty() ) {
		return;
	}

	const auto components = computeComponents( graph );

	float currentYOffset = 0.0f;

	for ( const auto& component : components ) {
		applyLayoutToComponent( graph, component, currentYOffset );

		float maxY = currentYOffset;
		for ( int nodeId : component ) {
			const Node* node = graph.findNode( nodeId );
			if ( node != nullptr ) {
				maxY = std::max( maxY, node->y + node->height );
			}
		}

		currentYOffset = maxY + config_.component_spacing;
	}
}

void LayoutEngine::applyLayoutToComponent( Graph& graph, const Component& component, float y_offset ) const {
	std::unordered_set< int > componentSet( component.begin(), component.end() );
	AdjList outgoing;
	AdjList incoming;

	for ( int id : component ) {
		outgoing[ id ] = {};
		incoming[ id ] = {};
	}

	for ( const auto& edge : graph.getEdges() ) {
		if ( componentSet.contains( edge.from ) && componentSet.contains( edge.to ) ) {
			outgoing[ edge.from ].push_back( edge.to );
			incoming[ edge.to ].push_back( edge.from );
		}
	}

	auto layerMap = computeLayersInternal( graph, component, outgoing );
	arrangeNodesInLayers( graph, layerMap, incoming, y_offset );
}

void LayoutEngine::arrangeNodesInLayers( Graph& graph, const LayerMap& layer, const AdjList& incoming,
                                         float y_offset ) const {
	std::map< int, std::vector< Node* > > layers;

	for ( const auto& [ id, _ ] : graph.getNodes() ) {
		if ( !layer.contains( id ) ) {
			continue;
		}

		if ( Node* ptr = graph.findNode( id ) ) {
			layers[ layer.at( id ) ].push_back( ptr );
		}
	}

	for ( auto& [ layer_idx, nodes_in_layer ] : layers ) {
		std::ranges::sort( nodes_in_layer, [ &graph, &incoming ]( const Node* a, const Node* b ) {
			const float baryA = computeBarycenter( *a, graph, incoming );
			const float baryB = computeBarycenter( *b, graph, incoming );

			if ( baryA != baryB ) {
				return baryA < baryB;
			}

			return a->id < b->id;
		} );

		const float current_x = config_.margin_x + ( static_cast< float >( layer_idx ) * config_.layer_spacing );

		float current_y = config_.margin_y + y_offset;
		for ( Node* node : nodes_in_layer ) {
			node->x = current_x;
			node->y = current_y;

			current_y += node->height + config_.node_spacing;
		}
	}
}

}  // namespace task2