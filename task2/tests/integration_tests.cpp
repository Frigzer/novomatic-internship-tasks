#include "json_io.hpp"
#include "layout_engine.hpp"
#include "paths.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace task2 {
namespace {

std::filesystem::path makeTempPath( const std::string& stem ) {
	const auto uniqueId = std::chrono::steady_clock::now().time_since_epoch().count();
	return std::filesystem::temp_directory_path() /
	       ( "task2_integration_" + stem + "_" + std::to_string( uniqueId ) + ".json" );
}

struct InputExpectation {
	const char* fileName;
	std::size_t nodeCount;
	std::size_t edgeCount;
};

std::pair< float, float > verticalRange( const Graph& graph, const std::vector< int >& ids ) {
	float minY = std::numeric_limits< float >::max();
	float maxY = std::numeric_limits< float >::lowest();

	for ( int id : ids ) {
		const Node* node = graph.findNode( id );
		EXPECT_NE( node, nullptr );
		if ( node == nullptr ) {
			continue;
		}

		minY = std::min( minY, node->y );
		maxY = std::max( maxY, node->y + node->height );
	}

	return { minY, maxY };
}

std::vector< std::pair< int, int > > sortedEdges( const Graph& graph ) {
	std::vector< std::pair< int, int > > edges;
	edges.reserve( graph.getEdges().size() );

	for ( const auto& edge : graph.getEdges() ) {
		edges.emplace_back( edge.from, edge.to );
	}

	std::ranges::sort( edges );
	return edges;
}

}  // namespace

TEST( IntegrationTests, AllBundledInputGraphsLoadWithExpectedSizes ) {
	const std::array< InputExpectation, 8 > cases{ {
	    { .fileName = "branch.json", .nodeCount = 4U, .edgeCount = 3U },
	    { .fileName = "chain.json", .nodeCount = 4U, .edgeCount = 3U },
	    { .fileName = "cycle.json", .nodeCount = 4U, .edgeCount = 4U },
	    { .fileName = "dense_graph.json", .nodeCount = 6U, .edgeCount = 8U },
	    { .fileName = "diamond.json", .nodeCount = 5U, .edgeCount = 5U },
	    { .fileName = "long_labels.json", .nodeCount = 4U, .edgeCount = 3U },
	    { .fileName = "sample_graph.json", .nodeCount = 4U, .edgeCount = 3U },
	    { .fileName = "two_components.json", .nodeCount = 7U, .edgeCount = 5U },
	} };

	for ( const auto& testCase : cases ) {
		const auto graph = JsonGraphIO::loadFromFile( paths::inputDir / testCase.fileName );
		EXPECT_EQ( graph.getNodes().size(), testCase.nodeCount ) << testCase.fileName;
		EXPECT_EQ( graph.getEdges().size(), testCase.edgeCount ) << testCase.fileName;
	}
}

TEST( IntegrationTests, LayoutAfterLoadingDiamondGraphPlacesEdgesForward ) {
	Graph graph = JsonGraphIO::loadFromFile( paths::inputDir / "diamond.json" );

	LayoutEngine engine;
	engine.applyLayout( graph );

	const Node* n1 = graph.findNode( 1 );
	const Node* n2 = graph.findNode( 2 );
	const Node* n3 = graph.findNode( 3 );
	const Node* n4 = graph.findNode( 4 );
	const Node* n5 = graph.findNode( 5 );
	ASSERT_NE( n1, nullptr );
	ASSERT_NE( n2, nullptr );
	ASSERT_NE( n3, nullptr );
	ASSERT_NE( n4, nullptr );
	ASSERT_NE( n5, nullptr );

	EXPECT_LT( n1->x, n2->x );
	EXPECT_LT( n1->x, n3->x );
	EXPECT_LT( n2->x, n4->x );
	EXPECT_LT( n3->x, n4->x );
	EXPECT_LT( n4->x, n5->x );
}

TEST( IntegrationTests, LoadLayoutSaveAndReloadPreservesStructureForDenseGraph ) {
	Graph graph = JsonGraphIO::loadFromFile( paths::inputDir / "dense_graph.json" );

	LayoutEngine::Config config;
	config.margin_x      = 33.0f;
	config.margin_y      = 44.0f;
	config.layer_spacing = 111.0f;
	config.node_spacing  = 22.0f;

	LayoutEngine engine( config );
	engine.applyLayout( graph );

	const auto outputPath = makeTempPath( "dense_roundtrip" );
	JsonGraphIO::saveToFile( graph, outputPath );
	Graph loaded = JsonGraphIO::loadFromFile( outputPath );

	EXPECT_EQ( loaded.getNodes().size(), graph.getNodes().size() );
	EXPECT_EQ( loaded.getEdges().size(), graph.getEdges().size() );

	for ( const auto& [ id, original ] : graph.getNodes() ) {
		const Node* reloaded = loaded.findNode( id );
		ASSERT_NE( reloaded, nullptr );
		EXPECT_EQ( reloaded->name, original.name );
		EXPECT_FLOAT_EQ( reloaded->x, original.x );
		EXPECT_FLOAT_EQ( reloaded->y, original.y );
		EXPECT_FLOAT_EQ( reloaded->width, original.width );
		EXPECT_FLOAT_EQ( reloaded->height, original.height );
	}

	EXPECT_EQ( sortedEdges( loaded ), sortedEdges( graph ) );

	std::filesystem::remove( outputPath );
}

TEST( IntegrationTests, ApplyingLayoutToLoadedTwoComponentGraphSeparatesComponents ) {
	Graph graph = JsonGraphIO::loadFromFile( paths::inputDir / "two_components.json" );

	LayoutEngine::Config config;
	config.component_spacing = 120.0f;
	config.margin_y          = 30.0f;

	LayoutEngine engine( config );
	engine.applyLayout( graph );

	auto firstRange  = verticalRange( graph, { 1, 2, 3 } );
	auto secondRange = verticalRange( graph, { 10, 11, 12, 13 } );
	std::array< std::pair< float, float >, 2 > ranges{ firstRange, secondRange };
	std::ranges::sort( ranges, []( const auto& lhs, const auto& rhs ) { return lhs.first < rhs.first; } );

	EXPECT_GE( ranges[ 1 ].first - ranges[ 0 ].second, config.component_spacing );
}

TEST( IntegrationTests, ApplyingLayoutAssignsFiniteCoordinatesToEveryNodeInAllBundledInputs ) {
	const std::array< const char*, 8 > fileNames{ {
	    "branch.json",
	    "chain.json",
	    "cycle.json",
	    "dense_graph.json",
	    "diamond.json",
	    "long_labels.json",
	    "sample_graph.json",
	    "two_components.json",
	} };

	LayoutEngine::Config config;
	config.margin_x = 27.0f;
	config.margin_y = 31.0f;

	LayoutEngine engine( config );

	for ( const char* fileName : fileNames ) {
		Graph graph = JsonGraphIO::loadFromFile( paths::inputDir / fileName );
		engine.applyLayout( graph );

		for ( const auto& [ id, node ] : graph.getNodes() ) {
			EXPECT_TRUE( std::isfinite( node.x ) ) << fileName << " node " << id;
			EXPECT_TRUE( std::isfinite( node.y ) ) << fileName << " node " << id;
			EXPECT_GE( node.x, config.margin_x ) << fileName << " node " << id;
			EXPECT_GE( node.y, config.margin_y ) << fileName << " node " << id;
		}
	}
}

TEST( IntegrationTests, LayoutKeepsCustomNodeDimensionsForBundledLongLabelsGraph ) {
	Graph graph = JsonGraphIO::loadFromFile( paths::inputDir / "long_labels.json" );

	LayoutEngine::Config config;
	config.margin_x      = 60.0f;
	config.margin_y      = 40.0f;
	config.layer_spacing = 260.0f;

	LayoutEngine engine( config );
	engine.applyLayout( graph );

	const Node* node1 = graph.findNode( 1 );
	const Node* node2 = graph.findNode( 2 );
	const Node* node3 = graph.findNode( 3 );
	const Node* node4 = graph.findNode( 4 );
	ASSERT_NE( node1, nullptr );
	ASSERT_NE( node2, nullptr );
	ASSERT_NE( node3, nullptr );
	ASSERT_NE( node4, nullptr );

	EXPECT_FLOAT_EQ( node1->width, 320.0f );
	EXPECT_FLOAT_EQ( node2->width, 360.0f );
	EXPECT_FLOAT_EQ( node3->width, 360.0f );
	EXPECT_FLOAT_EQ( node4->width, 380.0f );
	EXPECT_FLOAT_EQ( node1->height, 90.0f );
	EXPECT_FLOAT_EQ( node2->height, 90.0f );
	EXPECT_FLOAT_EQ( node3->height, 90.0f );
	EXPECT_FLOAT_EQ( node4->height, 90.0f );
}

}  // namespace task2
