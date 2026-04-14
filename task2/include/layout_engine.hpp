#pragma once

#include "graph.hpp"

namespace task2 {

class LayoutEngine {
public:
	struct Config {
		float margin_x{ 100.0f };
		float margin_y{ 100.0f };
		float layer_spacing{ 250.0f };
		float node_spacing{ 140.0f };
		float component_spacing{ 200.0f };
	};

	LayoutEngine();
	explicit LayoutEngine( const Config& config );
	void applyLayout( Graph& graph ) const;

private:
	using LayerMap  = std::unordered_map< int, int >;
	using AdjList   = std::unordered_map< int, std::vector< int > >;
	using Component = std::vector< int >;

	void arrangeNodesInLayers( Graph& graph, const LayerMap& layer, const AdjList& incoming, float y_offset ) const;

	std::vector< Component > computeComponents( const Graph& graph ) const;
	void applyLayoutToComponent( Graph& graph, const Component& component, float y_offset ) const;

	Config config_;
};

}  // namespace task2