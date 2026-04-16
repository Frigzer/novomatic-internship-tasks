#include "graph.hpp"

#include <gtest/gtest.h>

namespace task2 {

TEST( GraphTests, AddNodeReturnsTrueOnlyForUniqueIds ) {
	Graph graph;

	EXPECT_TRUE( graph.addNode( { .id = 1, .name = "First" } ) );
	EXPECT_FALSE( graph.addNode( { .id = 1, .name = "Duplicate" } ) );

	ASSERT_EQ( graph.getNodes().size(), 1U );
	EXPECT_EQ( graph.findNode( 1 )->name, "First" );
}

TEST( GraphTests, AddEdgeFailsWhenEndpointIsMissing ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "Only node" } );

	EXPECT_FALSE( graph.addEdge( { .from = 1, .to = 99 } ) );
	EXPECT_FALSE( graph.addEdge( { .from = 77, .to = 1 } ) );
	EXPECT_TRUE( graph.getEdges().empty() );
}

TEST( GraphTests, AddEdgeStoresValidConnectionsInInsertionOrder ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "A" } );
	graph.addNode( { .id = 2, .name = "B" } );
	graph.addNode( { .id = 3, .name = "C" } );

	ASSERT_TRUE( graph.addEdge( { .from = 1, .to = 2 } ) );
	ASSERT_TRUE( graph.addEdge( { .from = 2, .to = 3 } ) );

	ASSERT_EQ( graph.getEdges().size(), 2U );
	EXPECT_EQ( graph.getEdges()[ 0 ].from, 1 );
	EXPECT_EQ( graph.getEdges()[ 0 ].to, 2 );
	EXPECT_EQ( graph.getEdges()[ 1 ].from, 2 );
	EXPECT_EQ( graph.getEdges()[ 1 ].to, 3 );
}

TEST( GraphTests, FindNodeReturnsMutablePointerForExistingNode ) {
	Graph graph;
	graph.addNode( { .id = 7, .name = "Editable", .x = 10.0f, .y = 20.0f } );

	Node* node = graph.findNode( 7 );
	ASSERT_NE( node, nullptr );

	node->name = "Changed";
	node->x    = 42.0f;

	const Graph& constGraph   = graph;
	const Node* constNodeView = constGraph.findNode( 7 );
	ASSERT_NE( constNodeView, nullptr );
	EXPECT_EQ( constNodeView->name, "Changed" );
	EXPECT_FLOAT_EQ( constNodeView->x, 42.0f );
}

TEST( GraphTests, HasNodeAndFindNodeReportMissingIds ) {
	Graph graph;
	graph.addNode( { .id = 3, .name = "Existing" } );

	EXPECT_TRUE( graph.hasNode( 3 ) );
	EXPECT_FALSE( graph.hasNode( 999 ) );
	EXPECT_EQ( graph.findNode( 999 ), nullptr );
}

TEST( GraphTests, ClearRemovesAllNodesAndEdges ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "A" } );
	graph.addNode( { .id = 2, .name = "B" } );
	ASSERT_TRUE( graph.addEdge( { .from = 1, .to = 2 } ) );

	graph.clear();

	EXPECT_TRUE( graph.getNodes().empty() );
	EXPECT_TRUE( graph.getEdges().empty() );
	EXPECT_EQ( graph.findNode( 1 ), nullptr );
}

TEST( GraphTests, HandlesSelfLoopEdge ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "Loop" } );

	EXPECT_TRUE( graph.addEdge( { .from = 1, .to = 1 } ) );
	ASSERT_EQ( graph.getEdges().size(), 1U );
	EXPECT_EQ( graph.getEdges().front().from, 1 );
	EXPECT_EQ( graph.getEdges().front().to, 1 );
}

}  // namespace task2
