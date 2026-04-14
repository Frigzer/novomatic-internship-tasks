#pragma once

#include "graph.hpp"

#include <filesystem>

namespace task2 {

class JsonGraphIO {
public:
	static Graph loadFromFile( const std::filesystem::path& path );
	static void saveToFile( const Graph& graph, const std::filesystem::path& path );

private:
	static void validateGraph( const Graph& graph );
};

}  // namespace task2