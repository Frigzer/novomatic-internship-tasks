#pragma once

#include "graph.hpp"

#include <SFML/Graphics.hpp>

namespace task2 {

class GraphRenderer {
public:
	void draw( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const;

private:
	void drawEdges( sf::RenderTarget& target, const Graph& graph ) const;
	void drawNodes( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const;
	void drawSingleNode( sf::RenderTarget& target, const Node& node, const sf::Font& font ) const;
	void drawSingleEdge( sf::RenderTarget& target, const Node& from, const Node& to ) const;
};

}  // namespace task2