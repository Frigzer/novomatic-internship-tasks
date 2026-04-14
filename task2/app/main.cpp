#include "json_io.hpp"
#include "layout_engine.hpp"
#include "paths.hpp"

#include <exception>
#include <iostream>

int main() {
	try {
		const auto inputPath = task2::paths::dataDir() / "sample_graph.json";
		auto graph           = task2::JsonGraphIO::loadFromFile( inputPath.string() );

		task2::LayoutEngine engine;
		engine.applyLayout( graph );

		const auto outputPath = task2::paths::dataDir() / "sample_graph_out.json";
		task2::JsonGraphIO::saveToFile( graph, outputPath.string() );

		std::cout << "Layout applied successfully.\n";
		std::cout << "Output saved to: task2/data/sample_graph_out.json\n";
	} catch ( const std::exception& ex ) {
		std::cerr << "Error: " << ex.what() << '\n';
		return 1;
	}

	return 0;
}