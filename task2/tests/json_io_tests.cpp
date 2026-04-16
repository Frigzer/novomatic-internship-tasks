#include "json_io.hpp"
#include "paths.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace task2 {
namespace {

std::filesystem::path makeTempPath( const std::string& stem ) {
	const auto uniqueId = std::chrono::steady_clock::now().time_since_epoch().count();
	return std::filesystem::temp_directory_path() / ( "task2_" + stem + "_" + std::to_string( uniqueId ) + ".json" );
}

std::filesystem::path writeJsonFile( const std::string& stem, const std::string& content ) {
	const auto path = makeTempPath( stem );
	std::ofstream output( path );
	output << content;
	return path;
}

nlohmann::json readJsonFile( const std::filesystem::path& path ) {
	std::ifstream input( path );
	nlohmann::json json;
	input >> json;
	return json;
}

}  // namespace

TEST( JsonGraphIOTests, LoadsValidGraphFromSampleFile ) {
	const auto inputPath = task2::paths::inputDir / "sample_graph.json";
	const auto graph     = task2::JsonGraphIO::loadFromFile( inputPath );

	EXPECT_EQ( graph.getNodes().size(), 4U );
	EXPECT_EQ( graph.getEdges().size(), 3U );
}

TEST( JsonGraphIOTests, UsesDefaultValuesForMissingOptionalNodeFields ) {
	const auto path = writeJsonFile(
	    "defaults",
	    R"({
	        "nodes": [
	            { "id": 1, "name": "Start" }
	        ],
	        "edges": []
	    })" );

	const auto graph = JsonGraphIO::loadFromFile( path );
	const Node* node = graph.findNode( 1 );

	ASSERT_NE( node, nullptr );
	EXPECT_FLOAT_EQ( node->x, 0.0f );
	EXPECT_FLOAT_EQ( node->y, 0.0f );
	EXPECT_FLOAT_EQ( node->width, Node::DefaultWidth );
	EXPECT_FLOAT_EQ( node->height, Node::DefaultHeight );

	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, ThrowsWhenNodesArrayIsMissing ) {
	const auto path = writeJsonFile(
	    "missing_nodes",
	    R"({
	        "edges": []
	    })" );

	EXPECT_THROW( JsonGraphIO::loadFromFile( path ), std::runtime_error );
	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, ThrowsWhenEdgesArrayIsMissing ) {
	const auto path = writeJsonFile(
	    "missing_edges",
	    R"({
	        "nodes": [
	            { "id": 1, "name": "Start" }
	        ]
	    })" );

	EXPECT_THROW( JsonGraphIO::loadFromFile( path ), std::runtime_error );
	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, ThrowsWhenNodeNameIsEmpty ) {
	const auto path = writeJsonFile(
	    "empty_name",
	    R"({
	        "nodes": [
	            { "id": 1, "name": "" }
	        ],
	        "edges": []
	    })" );

	EXPECT_THROW( JsonGraphIO::loadFromFile( path ), std::runtime_error );
	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, ThrowsWhenNodeWidthIsNonPositive ) {
	const auto path = writeJsonFile(
	    "bad_width",
	    R"({
	        "nodes": [
	            { "id": 1, "name": "Bad", "width": 0.0 }
	        ],
	        "edges": []
	    })" );

	EXPECT_THROW( JsonGraphIO::loadFromFile( path ), std::runtime_error );
	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, ThrowsWhenNodeHeightIsNonPositive ) {
	const auto path = writeJsonFile(
	    "bad_height",
	    R"({
	        "nodes": [
	            { "id": 1, "name": "Bad", "height": -5.0 }
	        ],
	        "edges": []
	    })" );

	EXPECT_THROW( JsonGraphIO::loadFromFile( path ), std::runtime_error );
	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, ThrowsWhenDuplicateNodeIdsAppearInJson ) {
	const auto path = writeJsonFile(
	    "duplicate_nodes",
	    R"({
	        "nodes": [
	            { "id": 1, "name": "First" },
	            { "id": 1, "name": "Second" }
	        ],
	        "edges": []
	    })" );

	EXPECT_THROW( JsonGraphIO::loadFromFile( path ), std::runtime_error );
	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, ThrowsWhenEdgeReferencesMissingNode ) {
	const auto path = writeJsonFile(
	    "orphan_edge",
	    R"({
	        "nodes": [
	            { "id": 1, "name": "Start" }
	        ],
	        "edges": [
	            { "from": 1, "to": 2 }
	        ]
	    })" );

	EXPECT_THROW( JsonGraphIO::loadFromFile( path ), std::runtime_error );
	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, SaveToFileSortsNodesAndEdgesById ) {
	Graph graph;
	graph.addNode( { .id = 5, .name = "Five", .x = 50.0f, .y = 10.0f } );
	graph.addNode( { .id = 2, .name = "Two", .x = 20.0f, .y = 30.0f } );
	graph.addNode( { .id = 9, .name = "Nine", .x = 90.0f, .y = 70.0f } );

	ASSERT_TRUE( graph.addEdge( { .from = 5, .to = 9 } ) );
	ASSERT_TRUE( graph.addEdge( { .from = 2, .to = 5 } ) );
	ASSERT_TRUE( graph.addEdge( { .from = 2, .to = 9 } ) );

	const auto path = makeTempPath( "sorted_output" );
	JsonGraphIO::saveToFile( graph, path );

	const auto json = readJsonFile( path );
	ASSERT_EQ( json[ "nodes" ].size(), 3U );
	ASSERT_EQ( json[ "edges" ].size(), 3U );

	EXPECT_EQ( json[ "nodes" ][ 0 ][ "id" ].get< int >(), 2 );
	EXPECT_EQ( json[ "nodes" ][ 1 ][ "id" ].get< int >(), 5 );
	EXPECT_EQ( json[ "nodes" ][ 2 ][ "id" ].get< int >(), 9 );

	EXPECT_EQ( json[ "edges" ][ 0 ][ "from" ].get< int >(), 2 );
	EXPECT_EQ( json[ "edges" ][ 0 ][ "to" ].get< int >(), 5 );
	EXPECT_EQ( json[ "edges" ][ 1 ][ "from" ].get< int >(), 2 );
	EXPECT_EQ( json[ "edges" ][ 1 ][ "to" ].get< int >(), 9 );
	EXPECT_EQ( json[ "edges" ][ 2 ][ "from" ].get< int >(), 5 );
	EXPECT_EQ( json[ "edges" ][ 2 ][ "to" ].get< int >(), 9 );

	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, SaveToFileRoundTripsNodeGeometryAndEdges ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "Start", .x = 10.0f, .y = 20.0f, .width = 210.0f, .height = 95.0f } );
	graph.addNode( { .id = 2, .name = "End", .x = 300.0f, .y = 120.0f, .width = 180.0f, .height = 80.0f } );
	ASSERT_TRUE( graph.addEdge( { .from = 1, .to = 2 } ) );

	const auto path = makeTempPath( "roundtrip" );
	JsonGraphIO::saveToFile( graph, path );
	const auto loaded = JsonGraphIO::loadFromFile( path );

	ASSERT_EQ( loaded.getNodes().size(), 2U );
	ASSERT_EQ( loaded.getEdges().size(), 1U );

	const Node* start = loaded.findNode( 1 );
	const Node* end   = loaded.findNode( 2 );
	ASSERT_NE( start, nullptr );
	ASSERT_NE( end, nullptr );

	EXPECT_EQ( start->name, "Start" );
	EXPECT_FLOAT_EQ( start->x, 10.0f );
	EXPECT_FLOAT_EQ( start->y, 20.0f );
	EXPECT_FLOAT_EQ( start->width, 210.0f );
	EXPECT_FLOAT_EQ( start->height, 95.0f );

	EXPECT_EQ( end->name, "End" );
	EXPECT_FLOAT_EQ( end->x, 300.0f );
	EXPECT_FLOAT_EQ( end->y, 120.0f );
	EXPECT_FLOAT_EQ( end->width, 180.0f );
	EXPECT_FLOAT_EQ( end->height, 80.0f );

	EXPECT_EQ( loaded.getEdges().front().from, 1 );
	EXPECT_EQ( loaded.getEdges().front().to, 2 );

	std::filesystem::remove( path );
}

TEST( JsonGraphIOTests, SaveToFileRejectsInvalidGraphs ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "", .width = Node::DefaultWidth, .height = Node::DefaultHeight } );

	const auto path = makeTempPath( "invalid_save" );
	EXPECT_THROW( JsonGraphIO::saveToFile( graph, path ), std::runtime_error );
	std::filesystem::remove( path );
}

}  // namespace task2
