#pragma once

#include "graph.hpp"

#include <SFML/Graphics.hpp>
#include <string>

namespace task2 {

class GraphRenderer {
public:
	void draw( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const;

private:
	void drawGrid( sf::RenderTarget& target ) const;
	void drawEdges( sf::RenderTarget& target, const Graph& graph ) const;
	void drawNodes( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const;

	void drawSingleNode( sf::RenderTarget& target, const Node& node, const sf::Font& font ) const;
	void drawSingleEdge( sf::RenderTarget& target, const Node& from, const Node& to ) const;
	void drawArrowHead( sf::RenderTarget& target, sf::Vector2f tip, sf::Vector2f direction ) const;

	std::string fitTextToWidth( const std::string& text, const sf::Font& font, unsigned int characterSize,
	                            float maxWidth ) const;
};

}  // namespace task2