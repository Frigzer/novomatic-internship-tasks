#include "graph_renderer.hpp"

#include "visual_config.hpp"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

namespace task2 {
namespace {

sf::Vector2f normalize( sf::Vector2f vector ) {
	const float length = std::hypot( vector.x, vector.y );
	if ( length == 0.0f ) {
		return { 0.0f, 0.0f };
	}

	return { vector.x / length, vector.y / length };
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

	static const std::string ellipsis = "...";
	sf::Text ellipsisMeasure( font, ellipsis, characterSize );
	if ( ellipsisMeasure.getLocalBounds().size.x > maxWidth ) {
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

void colorizePath( sf::VertexArray& path, const sf::Color& color ) {
	for ( std::size_t index = 0; index < path.getVertexCount(); ++index ) {
		path[ index ].color = color;
	}
}

std::vector< std::reference_wrapper< const Node > > collectSortedNodes( const Graph& graph ) {
	std::vector< std::reference_wrapper< const Node > > sortedNodes;
	sortedNodes.reserve( graph.getNodes().size() );

	for ( const auto& [ _, node ] : graph.getNodes() ) {
		sortedNodes.push_back( std::cref( node ) );
	}

	std::ranges::sort( sortedNodes, []( const Node& left, const Node& right ) {
		if ( left.x != right.x ) {
			return left.x < right.x;
		}
		if ( left.y != right.y ) {
			return left.y < right.y;
		}
		return left.id < right.id;
	} );

	return sortedNodes;
}

void drawGrid( sf::RenderTarget& target ) {
	const sf::View view       = target.getView();
	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size   = view.getSize();

	const float left   = center.x - ( size.x * 0.5f );
	const float right  = center.x + ( size.x * 0.5f );
	const float top    = center.y - ( size.y * 0.5f );
	const float bottom = center.y + ( size.y * 0.5f );

	namespace VC = VisualConfig;

	sf::VertexArray lines( sf::PrimitiveType::Lines );
	auto appendLine = [ & ]( sf::Vector2f from, sf::Vector2f to, const sf::Color& color ) {
		lines.append( sf::Vertex( from, color ) );
		lines.append( sf::Vertex( to, color ) );
	};

	const float firstVertical = std::floor( left / VC::MinorGridStep ) * VC::MinorGridStep;
	for ( float x = firstVertical; x <= right; x += VC::MinorGridStep ) {
		const bool isMajor = std::fmod( std::abs( x ), VC::MajorGridStep ) < VC::GridTolerance ||
		                     std::fmod( std::abs( x ), VC::MajorGridStep ) > ( VC::MajorGridStep - VC::GridTolerance );

		appendLine( { x, top }, { x, bottom }, isMajor ? VC::ColorMajorGrid : VC::ColorMinorGrid );
	}

	const float firstHorizontal = std::floor( top / VC::MinorGridStep ) * VC::MinorGridStep;
	for ( float y = firstHorizontal; y <= bottom; y += VC::MinorGridStep ) {
		const bool isMajor = std::fmod( std::abs( y ), VC::MajorGridStep ) < VC::GridTolerance ||
		                     std::fmod( std::abs( y ), VC::MajorGridStep ) > ( VC::MajorGridStep - VC::GridTolerance );

		appendLine( { left, y }, { right, y }, isMajor ? VC::ColorMajorGrid : VC::ColorMinorGrid );
	}

	target.draw( lines );
}

void drawArrowHead( sf::RenderTarget& target, sf::Vector2f tip, sf::Vector2f direction ) {
	namespace VC = VisualConfig;

	direction = normalize( direction );
	const sf::Vector2f perpendicular{ -direction.y, direction.x };

	sf::ConvexShape arrow;
	arrow.setPointCount( 3 );
	arrow.setPoint( 0, tip );
	arrow.setPoint( 1, tip - direction * VC::ArrowLength + perpendicular * VC::ArrowWidth );
	arrow.setPoint( 2, tip - direction * VC::ArrowLength - perpendicular * VC::ArrowWidth );
	arrow.setFillColor( VC::ColorArrow );

	target.draw( arrow );
}

void drawSingleNode( sf::RenderTarget& target, const Node& node, const sf::Font& font ) {
	namespace VC = VisualConfig;

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

	const float maxTitleWidth     = std::max( 0.0f, node.width - ( 2.0f * VC::NodeHorizontalPadding ) );
	const std::string fittedTitle = fitTextToWidth( node.name, font, VC::NodeTitleSize, maxTitleWidth );

	sf::Text title( font, fittedTitle, VC::NodeTitleSize );
	title.setFillColor( sf::Color::White );
	title.setPosition( { node.x + VC::NodeHorizontalPadding, node.y + VC::NodeTitleTopMargin } );
	target.draw( title );

	sf::Text idText( font, "#" + std::to_string( node.id ), VC::NodeIdSize );
	idText.setFillColor( VC::ColorTextId );
	idText.setPosition( { node.x + VC::NodeHorizontalPadding, node.y + node.height - VC::NodeIdBottomOffset } );
	target.draw( idText );
}

void drawNodes( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) {
	for ( const auto& nodeRef : collectSortedNodes( graph ) ) {
		drawSingleNode( target, nodeRef.get(), font );
	}
}

void drawForwardEdge( sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end ) {
	namespace VC = VisualConfig;

	const float horizontalOffset = std::max( VC::EdgeMinHorizontalOffset, ( end.x - start.x ) * 0.5f );
	const float bendX            = start.x + horizontalOffset;

	sf::VertexArray path( sf::PrimitiveType::LineStrip, VC::PathPointsForward );
	path[ 0 ].position = start;
	path[ 1 ].position = { bendX, start.y };
	path[ 2 ].position = { bendX, end.y };
	path[ 3 ].position = end;
	colorizePath( path, VC::ColorEdge );

	target.draw( path );
	drawArrowHead( target, end, { 1.0f, 0.0f } );
}

void drawBackwardEdge( sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end ) {
	namespace VC = VisualConfig;

	const float detourY = std::min( start.y, end.y ) - VC::EdgeDetourYOffset;
	const float exitX   = start.x + VC::EdgeBackExitOffset;
	const float entryX  = end.x - VC::EdgeBackExitOffset;

	sf::VertexArray path( sf::PrimitiveType::LineStrip, VC::PathPointsBackward );
	path[ 0 ].position = start;
	path[ 1 ].position = { exitX, start.y };
	path[ 2 ].position = { exitX, detourY };
	path[ 3 ].position = { entryX, detourY };
	path[ 4 ].position = { entryX, end.y };
	path[ 5 ].position = end;  // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
	colorizePath( path, VC::ColorEdge );

	target.draw( path );
	drawArrowHead( target, end, { -1.0f, 0.0f } );
}

void drawSingleEdge( sf::RenderTarget& target, const Node& from, const Node& to ) {
	const sf::Vector2f start{ from.x + from.width, from.y + ( from.height * 0.5f ) };
	const sf::Vector2f end{ to.x, to.y + ( to.height * 0.5f ) };

	if ( end.x > start.x ) {
		drawForwardEdge( target, start, end );
		return;
	}

	drawBackwardEdge( target, start, end );
}

void drawEdges( sf::RenderTarget& target, const Graph& graph ) {
	for ( const auto& edge : graph.getEdges() ) {
		const Node* from = graph.findNode( edge.from );
		const Node* to   = graph.findNode( edge.to );

		if ( from == nullptr || to == nullptr ) {
			continue;
		}

		drawSingleEdge( target, *from, *to );
	}
}

}  // namespace

void GraphRenderer::draw( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) {
	drawGrid( target );
	drawEdges( target, graph );
	drawNodes( target, graph, font );
}

}  // namespace task2
