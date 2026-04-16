#include "layout_engine.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <limits>
#include <utility>
#include <vector>

namespace task2 {
namespace {

Graph makeBranchGraph() {
	Graph graph;
	graph.addNode( { .id = 1, .name = "Start" } );
	graph.addNode( { .id = 2, .name = "Branch" } );
	graph.addNode( { .id = 3, .name = "Left" } );
	graph.addNode( { .id = 4, .name = "Right" } );

	graph.addEdge( { .from = 1, .to = 2 } );
	graph.addEdge( { .from = 2, .to = 3 } );
	graph.addEdge( { .from = 2, .to = 4 } );

	return graph;
}

Graph makeChainGraph() {
	Graph graph;
	graph.addNode( { .id = 1, .name = "Start" } );
	graph.addNode( { .id = 2, .name = "Middle" } );
	graph.addNode( { .id = 3, .name = "End" } );

	graph.addEdge( { .from = 1, .to = 2 } );
	graph.addEdge( { .from = 2, .to = 3 } );

	return graph;
}

Graph makeTwoComponentGraph() {
	Graph graph;
	graph.addNode( { .id = 1, .name = "A1" } );
	graph.addNode( { .id = 2, .name = "A2" } );
	graph.addNode( { .id = 3, .name = "A3" } );
	graph.addNode( { .id = 10, .name = "B1" } );
	graph.addNode( { .id = 11, .name = "B2" } );
	graph.addNode( { .id = 12, .name = "B3" } );
	graph.addNode( { .id = 13, .name = "B4" } );

	graph.addEdge( { .from = 1, .to = 2 } );
	graph.addEdge( { .from = 2, .to = 3 } );
	graph.addEdge( { .from = 10, .to = 11 } );
	graph.addEdge( { .from = 11, .to = 12 } );
	graph.addEdge( { .from = 11, .to = 13 } );

	return graph;
}

std::pair< float, float > componentVerticalRange( const Graph& graph, const std::vector< int >& ids ) {
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

}  // namespace

TEST( LayoutEngineTests, LeavesEmptyGraphUntouched ) {
	Graph graph;

	LayoutEngine engine;
	EXPECT_NO_THROW( engine.applyLayout( graph ) );
	EXPECT_TRUE( graph.getNodes().empty() );
	EXPECT_TRUE( graph.getEdges().empty() );
}

TEST( LayoutEngineTests, AssignsConfiguredXCoordinatesForChainLayers ) {
	Graph graph = makeChainGraph();

	LayoutEngine::Config config;
	config.margin_x     = 25.0f;
	config.margin_y     = 35.0f;
	config.layer_spacing = 180.0f;
	config.node_spacing = 90.0f;

	LayoutEngine engine( config );
	engine.applyLayout( graph );

	const Node* n1 = graph.findNode( 1 );
	const Node* n2 = graph.findNode( 2 );
	const Node* n3 = graph.findNode( 3 );
	ASSERT_NE( n1, nullptr );
	ASSERT_NE( n2, nullptr );
	ASSERT_NE( n3, nullptr );

	EXPECT_FLOAT_EQ( n1->x, 25.0f );
	EXPECT_FLOAT_EQ( n2->x, 205.0f );
	EXPECT_FLOAT_EQ( n3->x, 385.0f );

	EXPECT_FLOAT_EQ( n1->y, 35.0f );
	EXPECT_FLOAT_EQ( n2->y, 35.0f );
	EXPECT_FLOAT_EQ( n3->y, 35.0f );
}

TEST( LayoutEngineTests, PlacesChildrenToTheRightOfParents ) {
	Graph graph = makeBranchGraph();

	LayoutEngine engine;
	engine.applyLayout( graph );

	const Node* n1 = graph.findNode( 1 );
	const Node* n2 = graph.findNode( 2 );
	const Node* n3 = graph.findNode( 3 );
	const Node* n4 = graph.findNode( 4 );

	ASSERT_NE( n1, nullptr );
	ASSERT_NE( n2, nullptr );
	ASSERT_NE( n3, nullptr );
	ASSERT_NE( n4, nullptr );

	EXPECT_LT( n1->x, n2->x );
	EXPECT_LT( n2->x, n3->x );
	EXPECT_LT( n2->x, n4->x );
}

TEST( LayoutEngineTests, UsesNodeHeightAndSpacingWhenStackingSiblings ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "Root" } );
	graph.addNode( { .id = 2, .name = "Upper child", .height = 50.0f } );
	graph.addNode( { .id = 3, .name = "Lower child", .height = 70.0f } );
	graph.addEdge( { .from = 1, .to = 2 } );
	graph.addEdge( { .from = 1, .to = 3 } );

	LayoutEngine::Config config;
	config.margin_x      = 10.0f;
	config.margin_y      = 20.0f;
	config.layer_spacing = 100.0f;
	config.node_spacing  = 30.0f;

	LayoutEngine engine( config );
	engine.applyLayout( graph );

	const Node* upper = graph.findNode( 2 );
	const Node* lower = graph.findNode( 3 );
	ASSERT_NE( upper, nullptr );
	ASSERT_NE( lower, nullptr );

	EXPECT_FLOAT_EQ( upper->x, 110.0f );
	EXPECT_FLOAT_EQ( lower->x, 110.0f );
	EXPECT_FLOAT_EQ( upper->y, 20.0f );
	EXPECT_FLOAT_EQ( lower->y, 100.0f );
	EXPECT_LT( upper->y, lower->y );
}

TEST( LayoutEngineTests, OrdersRootNodesByIdWhenTheyHaveNoIncomingEdges ) {
	Graph graph;
	graph.addNode( { .id = 5, .name = "Later root" } );
	graph.addNode( { .id = 2, .name = "Earlier root" } );
	graph.addNode( { .id = 8, .name = "Shared child" } );
	graph.addEdge( { .from = 5, .to = 8 } );
	graph.addEdge( { .from = 2, .to = 8 } );

	LayoutEngine::Config config;
	config.margin_y     = 15.0f;
	config.node_spacing = 25.0f;

	LayoutEngine engine( config );
	engine.applyLayout( graph );

	const Node* earlierRoot = graph.findNode( 2 );
	const Node* laterRoot   = graph.findNode( 5 );
	ASSERT_NE( earlierRoot, nullptr );
	ASSERT_NE( laterRoot, nullptr );

	EXPECT_FLOAT_EQ( earlierRoot->y, 15.0f );
	EXPECT_FLOAT_EQ( laterRoot->y, 120.0f );
	EXPECT_LT( earlierRoot->y, laterRoot->y );
}

TEST( LayoutEngineTests, SeparatesDisconnectedComponentsVertically ) {
	Graph graph = makeTwoComponentGraph();

	LayoutEngine::Config config;
	config.margin_y          = 40.0f;
	config.component_spacing = 90.0f;

	LayoutEngine engine( config );
	engine.applyLayout( graph );

	auto rangeA = componentVerticalRange( graph, { 1, 2, 3 } );
	auto rangeB = componentVerticalRange( graph, { 10, 11, 12, 13 } );

	std::array< std::pair< float, float >, 2 > orderedRanges{ rangeA, rangeB };
	std::ranges::sort( orderedRanges, []( const auto& lhs, const auto& rhs ) { return lhs.first < rhs.first; } );

	EXPECT_GE( orderedRanges[ 1 ].first - orderedRanges[ 0 ].second, config.component_spacing );
}

TEST( LayoutEngineTests, ProducesStableForwardLayersForCycleGraphs ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "Start" } );
	graph.addNode( { .id = 2, .name = "Update" } );
	graph.addNode( { .id = 3, .name = "Check" } );
	graph.addNode( { .id = 4, .name = "Loop back" } );

	graph.addEdge( { .from = 1, .to = 2 } );
	graph.addEdge( { .from = 2, .to = 3 } );
	graph.addEdge( { .from = 3, .to = 4 } );
	graph.addEdge( { .from = 4, .to = 2 } );

	LayoutEngine engine;
	engine.applyLayout( graph );

	const Node* n1 = graph.findNode( 1 );
	const Node* n2 = graph.findNode( 2 );
	const Node* n3 = graph.findNode( 3 );
	const Node* n4 = graph.findNode( 4 );
	ASSERT_NE( n1, nullptr );
	ASSERT_NE( n2, nullptr );
	ASSERT_NE( n3, nullptr );
	ASSERT_NE( n4, nullptr );

	EXPECT_LT( n1->x, n2->x );
	EXPECT_LT( n2->x, n3->x );
	EXPECT_LT( n3->x, n4->x );
}

}  // namespace task2
