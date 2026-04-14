#include <gtest/gtest.h>

#include "json_io.hpp"
#include "paths.hpp"

namespace task2 {

TEST( JsonGraphIOTests, LoadsValidGraphFromFile ) {
	const auto inputPath = task2::paths::dataDir() / "sample_graph.json";
	auto graph           = task2::JsonGraphIO::loadFromFile( inputPath.string() );

	EXPECT_EQ( graph.getNodes().size(), 4U );
	EXPECT_EQ( graph.getEdges().size(), 3U );
}

}  // namespace task2