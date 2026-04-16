#pragma once

#include "graph.hpp"

#include <SFML/Graphics.hpp>
#include <string>

namespace task2 {

class GraphRenderer {
public:
	struct VisualConfig {
		//
		static constexpr float MinorGridStep = 50.0f;
		static constexpr float MajorGridStep = 200.0f;
		static constexpr float GridTolerance = 0.001f;

		static inline const sf::Color ColorMajorGrid{ 50, 54, 62 };
		static inline const sf::Color ColorMinorGrid{ 36, 40, 46 };
		static inline const sf::Color ColorBackground{ 24, 28, 33 };

		//
		static constexpr float NodeOutlineThickness  = 2.0f;
		static constexpr float NodeHeaderHeight      = 26.0f;
		static constexpr float NodeShadowOffset      = 4.0f;
		static constexpr float NodeHorizontalPadding = 10.0f;
		static constexpr float NodeTitleTopMargin    = 6.0f;
		static constexpr float NodeIdBottomOffset    = 22.0f;
		static constexpr unsigned int NodeTitleSize  = 16;
		static constexpr unsigned int NodeIdSize     = 12;

		static inline const sf::Color ColorNodeBody{ 42, 47, 56 };
		static inline const sf::Color ColorNodeHeader{ 58, 66, 78 };
		static inline const sf::Color ColorNodeOutline{ 95, 170, 255 };
		static inline const sf::Color ColorNodeShadow{ 0, 0, 0, 80 };
		static inline const sf::Color ColorTextId{ 190, 190, 190 };

		// Edge
		static constexpr float EdgeMinHorizontalOffset = 40.0f;
		static constexpr float EdgeDetourYOffset       = 80.0f;
		static constexpr float EdgeBackExitOffset      = 40.0f;
		static inline const sf::Color ColorEdge        = sf::Color( 210, 210, 210 );

		// Arrow
		static constexpr float ArrowLength = 10.0f;
		static constexpr float ArrowWidth  = 5.0f;
		static inline const sf::Color ColorArrow{ 220, 220, 220 };
	};

	void draw( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const;

private:
	void drawEdges( sf::RenderTarget& target, const Graph& graph ) const;
	void drawNodes( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const;

	void drawSingleNode( sf::RenderTarget& target, const Node& node, const sf::Font& font ) const;
	void drawSingleEdge( sf::RenderTarget& target, const Node& from, const Node& to ) const;
};

}  // namespace task2