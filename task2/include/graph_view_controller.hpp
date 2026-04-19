#pragma once

#include "graph.hpp"

#include <SFML/Graphics.hpp>

namespace task2 {

class GraphViewController {
public:
	static constexpr float DefaultFitPadding = 80.0f;

	explicit GraphViewController( const sf::View& initialView );

	[[nodiscard]] const sf::View& view() const noexcept;
	[[nodiscard]] sf::View& view() noexcept;

	void resetToDefault( const sf::RenderWindow& window );
	void resizeToWindow( const sf::Vector2u& windowSize );
	void zoomAtPixel( const sf::RenderWindow& window, float zoomFactor, const sf::Vector2i& pixel );

	void beginPanning( const sf::Vector2i& pixel ) noexcept;
	void endPanning() noexcept;
	void panTo( const sf::RenderWindow& window, const sf::Vector2i& pixel );

	void fitGraph( const sf::RenderWindow& window, const Graph& graph, float padding = DefaultFitPadding );
	[[nodiscard]] const Node* findHoveredNode( const sf::RenderWindow& window, const Graph& graph ) const;

private:
	sf::View view_;
	bool isPanning_{ false };
	sf::Vector2i lastMousePixel_;
};

}  // namespace task2
