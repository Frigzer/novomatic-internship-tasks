#pragma once

#include "graph.hpp"

namespace task2 {

class LayoutEngine {
public:
	using Component = std::vector< int >;
	using LayerMap  = std::unordered_map< int, int >;
	using AdjList   = std::unordered_map< int, std::vector< int > >;

	struct Config {
		static constexpr float DefaultMarginX          = 100.0f;
		static constexpr float DefaultMarginY          = 100.0f;
		static constexpr float DefaultLayerSpacing     = 250.0f;
		static constexpr float DefaultNodeSpacing      = 140.0f;
		static constexpr float DefaultComponentSpacing = 200.0f;

		float margin_x{ DefaultMarginX };
		float margin_y{ DefaultMarginY };
		float layer_spacing{ DefaultLayerSpacing };
		float node_spacing{ DefaultNodeSpacing };
		float component_spacing{ DefaultComponentSpacing };
	};

	LayoutEngine();
	explicit LayoutEngine( const Config& config );

	void applyLayout( Graph& graph ) const;

private:
	void applyLayoutToComponent( Graph& graph, const Component& component, float y_offset ) const;
	void arrangeNodesInLayers( Graph& graph, const LayerMap& layer, const AdjList& incoming, float y_offset ) const;

	Config config_;
};

}  // namespace task2