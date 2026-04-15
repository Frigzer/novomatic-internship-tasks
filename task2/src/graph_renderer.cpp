#include "graph_renderer.hpp"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace task2 {
namespace {

sf::Vector2f normalize( sf::Vector2f v ) {
	const float length = std::sqrt( v.x * v.x + v.y * v.y );
	if ( length == 0.0f ) {
		return { 0.0f, 0.0f };
	}
	return { v.x / length, v.y / length };
}

}  // namespace

void GraphRenderer::draw( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const {
	drawGrid( target );
	drawEdges( target, graph );
	drawNodes( target, graph, font );
}

void GraphRenderer::drawGrid( sf::RenderTarget& target ) const {
	const sf::View view       = target.getView();
	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size   = view.getSize();

	const float left   = center.x - size.x * 0.5f;
	const float right  = center.x + size.x * 0.5f;
	const float top    = center.y - size.y * 0.5f;
	const float bottom = center.y + size.y * 0.5f;

	constexpr float minorStep = 50.0f;
	constexpr float majorStep = 200.0f;

	sf::VertexArray lines( sf::PrimitiveType::Lines );

	auto appendLine = [ & ]( sf::Vector2f a, sf::Vector2f b, sf::Color color ) {
		lines.append( sf::Vertex( a, color ) );
		lines.append( sf::Vertex( b, color ) );
	};

	const float firstVertical = std::floor( left / minorStep ) * minorStep;
	for ( float x = firstVertical; x <= right; x += minorStep ) {
		const bool major = std::fmod( std::abs( x ), majorStep ) < 0.001f ||
		                   std::fmod( std::abs( x ), majorStep ) > ( majorStep - 0.001f );

		appendLine( { x, top }, { x, bottom }, major ? sf::Color( 50, 54, 62 ) : sf::Color( 36, 40, 46 ) );
	}

	const float firstHorizontal = std::floor( top / minorStep ) * minorStep;
	for ( float y = firstHorizontal; y <= bottom; y += minorStep ) {
		const bool major = std::fmod( std::abs( y ), majorStep ) < 0.001f ||
		                   std::fmod( std::abs( y ), majorStep ) > ( majorStep - 0.001f );

		appendLine( { left, y }, { right, y }, major ? sf::Color( 50, 54, 62 ) : sf::Color( 36, 40, 46 ) );
	}

	target.draw( lines );
}

void GraphRenderer::drawEdges( sf::RenderTarget& target, const Graph& graph ) const {
	for ( const auto& edge : graph.getEdges() ) {
		const Node* from = graph.findNode( edge.from );
		const Node* to   = graph.findNode( edge.to );

		if ( from == nullptr || to == nullptr ) {
			continue;
		}

		drawSingleEdge( target, *from, *to );
	}
}

void GraphRenderer::drawNodes( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const {
	std::vector< std::reference_wrapper< const Node > > sortedNodes;
	sortedNodes.reserve( graph.getNodes().size() );

	for ( const auto& [ _, node ] : graph.getNodes() ) {
		sortedNodes.push_back( std::cref( node ) );
	}

	std::ranges::sort( sortedNodes, []( const Node& a, const Node& b ) {
		if ( a.x != b.x ) {
			return a.x < b.x;
		}
		if ( a.y != b.y ) {
			return a.y < b.y;
		}
		return a.id < b.id;
	} );

	for ( const Node& node : sortedNodes ) {
		drawSingleNode( target, node, font );
	}
}

void GraphRenderer::drawSingleNode( sf::RenderTarget& target, const Node& node, const sf::Font& font ) const {
	sf::RectangleShape shadow;
	shadow.setPosition( { node.x + 4.0f, node.y + 4.0f } );
	shadow.setSize( { node.width, node.height } );
	shadow.setFillColor( sf::Color( 0, 0, 0, 80 ) );
	target.draw( shadow );

	sf::RectangleShape body;
	body.setPosition( { node.x, node.y } );
	body.setSize( { node.width, node.height } );
	body.setFillColor( sf::Color( 42, 47, 56 ) );
	body.setOutlineThickness( 2.0f );
	body.setOutlineColor( sf::Color( 95, 170, 255 ) );
	target.draw( body );

	sf::RectangleShape header;
	header.setPosition( { node.x, node.y } );
	header.setSize( { node.width, 26.0f } );
	header.setFillColor( sf::Color( 58, 66, 78 ) );
	target.draw( header );

	constexpr float horizontalPadding = 10.0f;
	constexpr unsigned int titleSize  = 16;

	const float maxTitleWidth     = std::max( 0.0f, node.width - 2.0f * horizontalPadding );
	const std::string fittedTitle = fitTextToWidth( node.name, font, titleSize, maxTitleWidth );

	sf::Text title( font, fittedTitle, titleSize );
	title.setFillColor( sf::Color::White );
	title.setPosition( { node.x + horizontalPadding, node.y + 6.0f } );
	target.draw( title );

	sf::Text idText( font, "#" + std::to_string( node.id ), 12 );
	idText.setFillColor( sf::Color( 190, 190, 190 ) );
	idText.setPosition( { node.x + 10.0f, node.y + node.height - 22.0f } );
	target.draw( idText );
}

void GraphRenderer::drawSingleEdge( sf::RenderTarget& target, const Node& from, const Node& to ) const {
	const sf::Vector2f start{ from.x + from.width, from.y + from.height * 0.5f };
	const sf::Vector2f end{ to.x, to.y + to.height * 0.5f };

	const float horizontalOffset = std::max( 40.0f, ( end.x - start.x ) * 0.5f );
	const float bendX            = start.x + horizontalOffset;

	sf::VertexArray path( sf::PrimitiveType::LineStrip, 4 );
	path[ 0 ].position = start;
	path[ 1 ].position = { bendX, start.y };
	path[ 2 ].position = { bendX, end.y };
	path[ 3 ].position = end;

	for ( std::size_t i = 0; i < path.getVertexCount(); ++i ) {
		path[ i ].color = sf::Color( 210, 210, 210 );
	}

	target.draw( path );

	drawArrowHead( target, end, { 1.0f, 0.0f } );
}

void GraphRenderer::drawArrowHead( sf::RenderTarget& target, sf::Vector2f tip, sf::Vector2f direction ) const {
	direction = normalize( direction );
	const sf::Vector2f perp{ -direction.y, direction.x };

	const float arrowLength = 10.0f;
	const float arrowWidth  = 5.0f;

	sf::ConvexShape arrow;
	arrow.setPointCount( 3 );
	arrow.setPoint( 0, tip );
	arrow.setPoint( 1, tip - direction * arrowLength + perp * arrowWidth );
	arrow.setPoint( 2, tip - direction * arrowLength - perp * arrowWidth );
	arrow.setFillColor( sf::Color( 220, 220, 220 ) );

	target.draw( arrow );
}

std::string GraphRenderer::fitTextToWidth( const std::string& text, const sf::Font& font, unsigned int characterSize,
                                           float maxWidth ) const {
	if ( text.empty() ) {
		return text;
	}

	sf::Text measure( font, text, characterSize );
	if ( measure.getLocalBounds().size.x <= maxWidth ) {
		return text;
	}

	const std::string ellipsis = "...";
	sf::Text ellipsisMeasure( font, ellipsis, characterSize );
	const float ellipsisWidth = ellipsisMeasure.getLocalBounds().size.x;

	if ( ellipsisWidth > maxWidth ) {
		return "";
	}

	std::string result = text;

	while ( !result.empty() ) {
		result.pop_back();

		sf::Text candidate( font, result + ellipsis, characterSize );
		if ( candidate.getLocalBounds().size.x <= maxWidth ) {
			return result + ellipsis;
		}
	}

	return "";
}

}  // namespace task2