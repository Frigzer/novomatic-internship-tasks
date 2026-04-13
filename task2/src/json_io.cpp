#include "json_io.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace task2 {

using json = nlohmann::json;

Graph JsonGraphIO::loadFromFile( const std::string& path ) {
	std::ifstream input( path );
	if ( !input ) {
		throw std::runtime_error( "Failed to open input file: " + path );
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
		node.width  = nodeJson.value( "width", 180.0f );
		node.height = nodeJson.value( "height", 80.0f );

		graph.nodes.push_back( std::move( node ) );
	}

	for ( const auto& edgeJson : j[ "edges" ] ) {
		Edge edge;
		edge.from = edgeJson.at( "from" ).get< int >();
		edge.to   = edgeJson.at( "to" ).get< int >();

		graph.edges.push_back( edge );
	}

	validateGraph( graph );
	return graph;
}

void JsonGraphIO::saveToFile( const Graph& graph, const std::string& path ) {
	validateGraph( graph );

	json j;
	j[ "nodes" ] = json::array();
	j[ "edges" ] = json::array();

	for ( const auto& node : graph.nodes ) {
		j[ "nodes" ].push_back( { { "id", node.id },
		                          { "name", node.name },
		                          { "x", node.x },
		                          { "y", node.y },
		                          { "width", node.width },
		                          { "height", node.height } } );
	}

	for ( const auto& edge : graph.edges ) {
		j[ "edges" ].push_back( { { "from", edge.from }, { "to", edge.to } } );
	}

	std::ofstream output( path );
	if ( !output ) {
		throw std::runtime_error( "Failed to open output file: " + path );
	}

	output << j.dump( 4 );
}

void JsonGraphIO::validateGraph( const Graph& graph ) {
	if ( !graph.hasUniqueNodeIds() ) {
		throw std::runtime_error( "Graph contains duplicate node ids" );
	}

	for ( const auto& node : graph.nodes ) {
		if ( node.name.empty() ) {
			throw std::runtime_error( "Graph contains a node with an empty name" );
		}
	}

	for ( const auto& edge : graph.edges ) {
		if ( !graph.hasNode( edge.from ) ) {
			throw std::runtime_error( "Edge references missing source node id: " + std::to_string( edge.from ) );
		}
		if ( !graph.hasNode( edge.to ) ) {
			throw std::runtime_error( "Edge references missing target node id: " + std::to_string( edge.to ) );
		}
	}
}

}  // namespace task2