#include "graph_view_controller.hpp"

#include <algorithm>
#include <limits>

namespace task2 {

GraphViewController::GraphViewController( const sf::View& initialView ) : view_( initialView ) {}

const sf::View& GraphViewController::view() const noexcept {
	return view_;
}

sf::View& GraphViewController::view() noexcept {
	return view_;
}

void GraphViewController::resetToDefault( const sf::RenderWindow& window ) {
	view_ = window.getDefaultView();
}

void GraphViewController::resizeToWindow( const sf::Vector2u& windowSize ) {
	if ( windowSize.x == 0U || windowSize.y == 0U ) {
		return;
	}

	const sf::Vector2f currentSize = view_.getSize();
	const float windowAspect       = static_cast< float >( windowSize.x ) / static_cast< float >( windowSize.y );
	const float viewAspect         = currentSize.x / currentSize.y;

	if ( viewAspect > windowAspect ) {
		view_.setSize( { currentSize.x, currentSize.x / windowAspect } );
		return;
	}

	view_.setSize( { currentSize.y * windowAspect, currentSize.y } );
}

void GraphViewController::zoomAtPixel( const sf::RenderWindow& window, float zoomFactor, const sf::Vector2i& pixel ) {
	const sf::Vector2f beforeZoom = window.mapPixelToCoords( pixel, view_ );
	view_.zoom( zoomFactor );
	const sf::Vector2f afterZoom = window.mapPixelToCoords( pixel, view_ );
	view_.move( beforeZoom - afterZoom );
}

void GraphViewController::beginPanning( const sf::Vector2i& pixel ) noexcept {
	isPanning_      = true;
	lastMousePixel_ = pixel;
}

void GraphViewController::endPanning() noexcept {
	isPanning_ = false;
}

void GraphViewController::panTo( const sf::RenderWindow& window, const sf::Vector2i& pixel ) {
	if ( !isPanning_ ) {
		return;
	}

	const sf::Vector2f previousWorld = window.mapPixelToCoords( lastMousePixel_, view_ );
	const sf::Vector2f currentWorld  = window.mapPixelToCoords( pixel, view_ );
	view_.move( previousWorld - currentWorld );
	lastMousePixel_ = pixel;
}

void GraphViewController::fitGraph( const sf::RenderWindow& window, const Graph& graph, float padding ) {
	const auto bounds = computeGraphBounds( graph, padding );
	if ( !bounds.has_value() ) {
		resetToDefault( window );
		return;
	}

	const float graphWidth  = std::max( 1.0f, bounds->maxX - bounds->minX );
	const float graphHeight = std::max( 1.0f, bounds->maxY - bounds->minY );

	view_.setCenter( { bounds->minX + ( graphWidth * 0.5f ), bounds->minY + ( graphHeight * 0.5f ) } );
	view_.setSize( { graphWidth, graphHeight } );
	resizeToWindow( window.getSize() );
}

const Node* GraphViewController::findHoveredNode( const sf::RenderWindow& window, const Graph& graph ) const {
	const sf::Vector2i mousePixel = sf::Mouse::getPosition( window );
	const sf::Vector2f mouseWorld = window.mapPixelToCoords( mousePixel, view_ );

	for ( const auto& [ _, node ] : graph.getNodes() ) {
		const sf::FloatRect bounds( { node.x, node.y }, { node.width, node.height } );
		if ( bounds.contains( mouseWorld ) ) {
			return &node;
		}
	}

	return nullptr;
}

std::optional< GraphViewController::GraphBounds > GraphViewController::computeGraphBounds( const Graph& graph,
                                                                                           float padding ) const {
	if ( graph.getNodes().empty() ) {
		return std::nullopt;
	}

	GraphBounds bounds{ .minX = std::numeric_limits< float >::max(),
	                    .minY = std::numeric_limits< float >::max(),
	                    .maxX = std::numeric_limits< float >::lowest(),
	                    .maxY = std::numeric_limits< float >::lowest() };

	for ( const auto& [ _, node ] : graph.getNodes() ) {
		bounds.minX = std::min( bounds.minX, node.x );
		bounds.minY = std::min( bounds.minY, node.y );
		bounds.maxX = std::max( bounds.maxX, node.x + node.width );
		bounds.maxY = std::max( bounds.maxY, node.y + node.height );
	}

	bounds.minX -= padding;
	bounds.minY -= padding;
	bounds.maxX += padding;
	bounds.maxY += padding;

	return bounds;
}

}  // namespace task2
