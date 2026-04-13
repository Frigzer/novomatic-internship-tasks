#include <gtest/gtest.h>

#include "layout_engine.hpp"

namespace task2 {
namespace {

Graph makeSampleGraph() {
	Graph graph;
	graph.nodes = {
	    { .id = 1, .name = "Event BeginPlay", .x = 0.0f, .y = 0.0f, .width = 180.0f, .height = 80.0f },
	    { .id = 2, .name = "Branch", .x = 0.0f, .y = 0.0f, .width = 180.0f, .height = 80.0f },
	    { .id = 3, .name = "Play Sound (True)", .x = 0.0f, .y = 0.0f, .width = 180.0f, .height = 80.0f },
	    { .id = 4, .name = "Spawn Actor (False)", .x = 0.0f, .y = 0.0f, .width = 180.0f, .height = 80.0f } };

	graph.edges = { { .from = 1, .to = 2 }, { .from = 2, .to = 3 }, { .from = 2, .to = 4 } };

	return graph;
}

}  // namespace

TEST( LayoutEngineTests, AssignsNonDefaultPositions ) {
	Graph graph = makeSampleGraph();

	LayoutEngine engine;
	engine.applyLayout( graph );

	for ( const auto& node : graph.nodes ) {
		EXPECT_TRUE( node.x != 0.0f || node.y != 0.0f );
	}
}

TEST( LayoutEngineTests, PlacesChildrenToTheRightOfParents ) {
	Graph graph = makeSampleGraph();

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

TEST( LayoutEngineTests, SeparatesNodesWithinTheSameLayerVertically ) {
	Graph graph = makeSampleGraph();

	LayoutEngine engine;
	engine.applyLayout( graph );

	const Node* n3 = graph.findNode( 3 );
	const Node* n4 = graph.findNode( 4 );

	ASSERT_NE( n3, nullptr );
	ASSERT_NE( n4, nullptr );

	EXPECT_NE( n3->y, n4->y );
}

}  // namespace task2