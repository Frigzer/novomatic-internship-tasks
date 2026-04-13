#pragma once

#include "graph.hpp"

#include <string>

namespace task2 {

class JsonGraphIO {
public:
	static Graph loadFromFile( const std::string& path );
	static void saveToFile( const Graph& graph, const std::string& path );

private:
	static void validateGraph( const Graph& graph );
};

}  // namespace task2