#include "graph_renderer.hpp"

#include <SFML/Graphics.hpp>

namespace task2 {

void GraphRenderer::draw( sf::RenderTarget& target, const Graph& graph, const sf::Font& font ) const {
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
	for ( const auto& [ _, node ] : graph.getNodes() ) {
		drawSingleNode( target, node, font );
	}
}

void GraphRenderer::drawSingleNode( sf::RenderTarget& target, const Node& node, const sf::Font& font ) const {
	sf::RectangleShape rect;
	rect.setPosition( { node.x, node.y } );
	rect.setSize( { node.width, node.height } );
	rect.setFillColor( sf::Color( 40, 44, 52 ) );
	rect.setOutlineThickness( 2.0f );
	rect.setOutlineColor( sf::Color( 100, 180, 255 ) );

	target.draw( rect );

	sf::Text text( font, node.name, 16 );
	text.setFillColor( sf::Color::White );
	text.setPosition( { node.x + 10.0f, node.y + 10.0f } );

	target.draw( text );

	sf::Text idText( font, "#" + std::to_string( node.id ), 12 );
	idText.setFillColor( sf::Color( 180, 180, 180 ) );
	idText.setPosition( { node.x + 10.0f, node.y + node.height - 22.0f } );

	target.draw( idText );
}

void GraphRenderer::drawSingleEdge( sf::RenderTarget& target, const Node& from, const Node& to ) const {
	const sf::Vector2f start{ from.x + from.width, from.y + from.height * 0.5f };
	const sf::Vector2f end{ to.x, to.y + to.height * 0.5f };

	const float midX = ( start.x + end.x ) * 0.5f;

	sf::VertexArray segments( sf::PrimitiveType::LineStrip, 4 );
	segments[ 0 ].position = start;
	segments[ 1 ].position = { midX, start.y };
	segments[ 2 ].position = { midX, end.y };
	segments[ 3 ].position = end;

	for ( std::size_t i = 0; i < segments.getVertexCount(); ++i ) {
		segments[ i ].color = sf::Color( 200, 200, 200 );
	}

	target.draw( segments );

	sf::CircleShape arrow( 4.0f );
	arrow.setFillColor( sf::Color( 220, 220, 220 ) );
	arrow.setPosition( { end.x - 4.0f, end.y - 4.0f } );
	target.draw( arrow );
}

}  // namespace task2