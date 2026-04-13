#include <gtest/gtest.h>

#include "json_io.hpp"
#include "paths.hpp"

namespace task2 {

TEST( JsonGraphIOTests, LoadsValidGraphFromFile ) {
	const auto inputPath = task2::paths::dataDir() / "sample_graph.json";
	auto graph           = task2::JsonGraphIO::loadFromFile( inputPath.string() );

	EXPECT_EQ( graph.nodes.size(), 4U );
	EXPECT_EQ( graph.edges.size(), 3U );
	EXPECT_TRUE( graph.hasUniqueNodeIds() );
}

}  // namespace task2