#include "json_io.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace task2 {

using json = nlohmann::json;

Graph JsonGraphIO::loadFromFile( const std::filesystem::path& path ) {
	std::ifstream input( path );
	if ( !input ) {
		throw std::runtime_error( "Failed to open input file: " + path.string() );
	}

	json j;
	input >> j;

	Graph graph;

	if ( !j.contains( "nodes" ) || !j[ "nodes" ].is_array() ) {
		throw std::runtime_error( "JSON must contain an array field: nodes" );
	}

	if ( !j.contains( "edges" ) || !j[ "edges" ].is_array() ) {
		throw std::runtime_error( "JSON must contain an array field: edges" );
	}

	for ( const auto& nodeJson : j[ "nodes" ] ) {
		Node node;
		node.id     = nodeJson.at( "id" ).get< int >();
		node.name   = nodeJson.at( "name" ).get< std::string >();
		node.x      = nodeJson.value( "x", 0.0f );
		node.y      = nodeJson.value( "y", 0.0f );
		node.width  = nodeJson.value( "width", Node::DefaultWidth );
		node.height = nodeJson.value( "height", Node::DefaultHeight );

		if ( node.name.empty() ) {
			throw std::runtime_error( "Node has an empty name. ID: " + std::to_string( node.id ) );
		}

		if ( node.width <= 0.0f ) {
			throw std::runtime_error( "Node has non-positive width. ID: " + std::to_string( node.id ) );
		}

		if ( node.height <= 0.0f ) {
			throw std::runtime_error( "Node has non-positive height. ID: " + std::to_string( node.id ) );
		}

		if ( !graph.addNode( std::move( node ) ) ) {
			throw std::runtime_error( "Duplicate node ID found in JSON: " + std::to_string( node.id ) );
		}
	}

	for ( const auto& edgeJson : j[ "edges" ] ) {
		Edge edge;
		edge.from = edgeJson.at( "from" ).get< int >();
		edge.to   = edgeJson.at( "to" ).get< int >();

		if ( !graph.addEdge( edge ) ) {
			throw std::runtime_error( "Edge references missing node(s): " + std::to_string( edge.from ) + " -> " +
			                          std::to_string( edge.to ) );
		}
	}

	validateGraph( graph );
	return graph;
}

void JsonGraphIO::saveToFile( const Graph& graph, const std::filesystem::path& path ) {
	validateGraph( graph );

	json j;
	j[ "nodes" ] = json::array();
	j[ "edges" ] = json::array();

	std::vector< std::reference_wrapper< const Node > > sortedNodes;
	sortedNodes.reserve( graph.getNodes().size() );

	for ( const auto& [ _, node ] : graph.getNodes() ) {
		sortedNodes.push_back( std::cref( node ) );
	}

	std::ranges::sort( sortedNodes, []( const Node& a, const Node& b ) { return a.id < b.id; } );

	for ( const Node& node : sortedNodes ) {
		j[ "nodes" ].push_back( { { "id", node.id },
		                          { "name", node.name },
		                          { "x", node.x },
		                          { "y", node.y },
		                          { "width", node.width },
		                          { "height", node.height } } );
	}

	auto sortedEdges = graph.getEdges();

	std::ranges::sort( sortedEdges, []( const Edge& a, const Edge& b ) {
		if ( a.from != b.from ) {
			return a.from < b.from;
		}
		return a.to < b.to;
	} );

	for ( const auto& edge : sortedEdges ) {
		j[ "edges" ].push_back( { { "from", edge.from }, { "to", edge.to } } );
	}

	std::ofstream output( path );
	if ( !output ) {
		throw std::runtime_error( "Failed to open output file: " + path.string() );
	}

	output << j.dump( 4 );
}

void JsonGraphIO::validateGraph( const Graph& graph ) {
	for ( const auto& [ id, node ] : graph.getNodes() ) {
		if ( node.name.empty() ) {
			throw std::runtime_error( "Graph contains a node with an empty name. ID: " + std::to_string( id ) );
		}

		if ( node.width <= 0.0f ) {
			throw std::runtime_error( "Graph contains a node with non-positive width. ID: " + std::to_string( id ) );
		}

		if ( node.height <= 0.0f ) {
			throw std::runtime_error( "Graph contains a node with non-positive height. ID: " + std::to_string( id ) );
		}
	}

	for ( const auto& edge : graph.getEdges() ) {
		if ( !graph.hasNode( edge.from ) || !graph.hasNode( edge.to ) ) {
			throw std::runtime_error( "Graph contains an orphan edge: " + std::to_string( edge.from ) + " -> " +
			                          std::to_string( edge.to ) );
		}
	}
}

}  // namespace task2