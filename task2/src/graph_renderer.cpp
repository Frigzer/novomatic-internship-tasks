#include "graph_renderer.hpp"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace task2 {
namespace {

sf::Vector2f normalize( sf::Vector2f v ) {
	const float length = std::sqrt( ( v.x * v.x ) + ( v.y * v.y ) );
	if ( length == 0.0f ) {
		return { 0.0f, 0.0f };
	}
	return { v.x / length, v.y / length };
}

void drawGrid( sf::RenderTarget& target ) {
	const sf::View view       = target.getView();
	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size   = view.getSize();

	const float left   = center.x - ( size.x * 0.5f );
	const float right  = center.x + ( size.x * 0.5f );
	const float top    = center.y - ( size.y * 0.5f );
	const float bottom = center.y + ( size.y * 0.5f );

	using VC = GraphRenderer::VisualConfig;

	sf::VertexArray lines( sf::PrimitiveType::Lines );

	auto appendLine = [ & ]( sf::Vector2f a, sf::Vector2f b, sf::Color color ) {
		lines.append( sf::Vertex( a, color ) );
		lines.append( sf::Vertex( b, color ) );
	};

	const float firstVertical = std::floor( left / VC::MinorGridStep ) * VC::MinorGridStep;
	for ( float x = firstVertical; x <= right; x += VC::MinorGridStep ) {
		const bool major = std::fmod( std::abs( x ), VC::MajorGridStep ) < VC::GridTolerance ||
		                   std::fmod( std::abs( x ), VC::MajorGridStep ) > ( VC::MajorGridStep - VC::GridTolerance );

		appendLine( { x, top }, { x, bottom }, major ? VC::ColorMajorGrid : VC::ColorMinorGrid );
	}

	const float firstHorizontal = std::floor( top / VC::MinorGridStep ) * VC::MinorGridStep;
	for ( float y = firstHorizontal; y <= bottom; y += VC::MinorGridStep ) {
		const bool major = std::fmod( std::abs( y ), VC::MajorGridStep ) < VC::GridTolerance ||
		                   std::fmod( std::abs( y ), VC::MajorGridStep ) > ( VC::MajorGridStep - VC::GridTolerance );

		appendLine( { left, y }, { right, y }, major ? VC::ColorMajorGrid : VC::ColorMinorGrid );
	}

	target.draw( lines );
}

void drawArrowHead( sf::RenderTarget& target, sf::Vector2f tip, sf::Vector2f direction ) {
	using VC = GraphRenderer::VisualConfig;

	direction = normalize( direction );
	const sf::Vector2f perp{ -direction.y, direction.x };

	const float arrowLength = 10.0f;
	const float arrowWidth  = 5.0f;

	sf::ConvexShape arrow;
	arrow.setPointCount( 3 );
	arrow.setPoint( 0, tip );
	arrow.setPoint( 1, tip - direction * VC::ArrowLength + perp * VC::ArrowWidth );
	arrow.setPoint( 2, tip - direction * VC::ArrowLength - perp * VC::ArrowWidth );
	arrow.setFillColor( VC::ColorArrow );

	target.draw( arrow );
}

std::string fitTextToWidth( const std::string& text, const sf::Font& font, unsigned int characterSize,
                            float maxWidth ) {
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

}  // namespace

void GraphRenderer::draw( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const {
	drawGrid( target );
	drawEdges( target, graph );
	drawNodes( target, graph, font );
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
	using VC = VisualConfig;

	sf::RectangleShape shadow;
	shadow.setPosition( { node.x + VC::NodeShadowOffset, node.y + VC::NodeShadowOffset } );
	shadow.setSize( { node.width, node.height } );
	shadow.setFillColor( VC::ColorNodeShadow );
	target.draw( shadow );

	sf::RectangleShape body;
	body.setPosition( { node.x, node.y } );
	body.setSize( { node.width, node.height } );
	body.setFillColor( VC::ColorNodeBody );
	body.setOutlineThickness( VC::NodeOutlineThickness );
	body.setOutlineColor( VC::ColorNodeOutline );
	target.draw( body );

	sf::RectangleShape header;
	header.setPosition( { node.x, node.y } );
	header.setSize( { node.width, VC::NodeHeaderHeight } );
	header.setFillColor( VC::ColorNodeHeader );
	target.draw( header );

	constexpr float horizontalPadding = 10.0f;
	constexpr unsigned int titleSize  = 16;

	const float maxTitleWidth     = std::max( 0.0f, node.width - ( 2.0f * VC::NodeHorizontalPadding ) );
	const std::string fittedTitle = fitTextToWidth( node.name, font, VC::NodeTitleSize, maxTitleWidth );

	sf::Text title( font, fittedTitle, titleSize );
	title.setFillColor( sf::Color::White );
	title.setPosition( { node.x + horizontalPadding, node.y + VC::NodeTitleTopMargin } );
	target.draw( title );

	sf::Text idText( font, "#" + std::to_string( node.id ), VC::NodeIdSize );
	idText.setFillColor( VC::ColorTextId );
	idText.setPosition( { node.x + VC::NodeHorizontalPadding, node.y + node.height - VC::NodeIdBottomOffset } );
	target.draw( idText );
}

void GraphRenderer::drawSingleEdge( sf::RenderTarget& target, const Node& from, const Node& to ) const {
	using VC = VisualConfig;

	const sf::Vector2f start{ from.x + from.width, from.y + ( from.height * 0.5f ) };
	const sf::Vector2f end{ to.x, to.y + ( to.height * 0.5f ) };

	const bool isBackwardEdge = end.x <= start.x;

	if ( !isBackwardEdge ) {
		const float horizontalOffset = std::max( 40.0f, ( end.x - start.x ) * 0.5f );
		const float bendX            = start.x + horizontalOffset;

		sf::VertexArray path( sf::PrimitiveType::LineStrip, 4 );
		path[ 0 ].position = start;
		path[ 1 ].position = { bendX, start.y };
		path[ 2 ].position = { bendX, end.y };
		path[ 3 ].position = end;

		for ( std::size_t i = 0; i < path.getVertexCount(); ++i ) {
			path[ i ].color = VC::ColorEdge;
		}

		target.draw( path );
		drawArrowHead( target, end, { 1.0f, 0.0f } );
		return;
	}

	// Back edge: prowadź połączenie nad nodami.
	const float detourY = std::min( start.y, end.y ) - 80.0f;
	const float exitX   = start.x + 40.0f;
	const float entryX  = end.x - 40.0f;

	sf::VertexArray path( sf::PrimitiveType::LineStrip, VC::EdgeDetourYOffset );
	path[ 0 ].position = start;
	path[ 1 ].position = { exitX, start.y };
	path[ 2 ].position = { exitX, detourY };
	path[ 3 ].position = { entryX, detourY };
	path[ 4 ].position = { entryX, end.y };
	path[ 5 ].position = end;

	for ( std::size_t i = 0; i < path.getVertexCount(); ++i ) {
		path[ i ].color = VC::ColorEdge;
	}

	target.draw( path );
	drawArrowHead( target, end, { -1.0f, 0.0f } );
}

}  // namespace task2