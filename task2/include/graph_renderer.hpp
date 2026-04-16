#pragma once

#include "graph.hpp"

#include <SFML/Graphics.hpp>

namespace task2::GraphRenderer {

void draw( sf::RenderTarget& target, const Graph& graph, const sf::Font& font );

}  // namespace task2::GraphRenderer
