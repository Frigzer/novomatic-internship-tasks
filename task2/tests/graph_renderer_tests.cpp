#include "graph_renderer.hpp"

#include "paths.hpp"
#include "visual_config.hpp"

#include <gtest/gtest.h>

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <stdexcept>

namespace task2 {
namespace {

sf::Image renderGraphToImage( const Graph& graph ) {
	sf::RenderTexture texture;
	if ( !texture.resize( { 800U, 600U } ) ) {
		throw std::runtime_error( "Failed to create render texture" );
	}

	texture.clear( VisualConfig::ColorBackground );

	sf::Font font;
	if ( !font.openFromFile( ( paths::dataDir / "font/arial.ttf" ).string() ) ) {
		throw std::runtime_error( "Failed to load test font" );
	}

	GraphRenderer::draw( texture, graph, font );
	texture.display();
	return texture.getTexture().copyToImage();
}

bool pixelEquals( const sf::Color& a, const sf::Color& b ) {
	return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

bool regionContainsColor( const sf::Image& image, unsigned int left, unsigned int top, unsigned int right,
                          unsigned int bottom, const sf::Color& color ) {
	const sf::Vector2u size = image.getSize();
	if ( size.x == 0 || size.y == 0 ) {
		return false;
	}

	left   = std::min( left, size.x - 1 );
	right  = std::min( right, size.x - 1 );
	top    = std::min( top, size.y - 1 );
	bottom = std::min( bottom, size.y - 1 );

	for ( unsigned int y = top; y <= bottom; ++y ) {
		for ( unsigned int x = left; x <= right; ++x ) {
			if ( pixelEquals( image.getPixel( { x, y } ), color ) ) {
				return true;
			}
		}
	}

	return false;
}

}  // namespace

TEST( GraphRendererTests, DrawsGridForEmptyGraph ) {
	const Graph graph;
	const sf::Image image = renderGraphToImage( graph );

	EXPECT_TRUE( regionContainsColor( image, 48U, 48U, 52U, 52U, VisualConfig::ColorMinorGrid ) );
	EXPECT_TRUE( regionContainsColor( image, 198U, 198U, 202U, 202U, VisualConfig::ColorMajorGrid ) );
}

TEST( GraphRendererTests, DrawsSingleNodeHeaderAndBodyColors ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "Renderer node", .x = 110.0f, .y = 90.0f } );

	const sf::Image image = renderGraphToImage( graph );

	EXPECT_TRUE( regionContainsColor( image, 260U, 100U, 270U, 110U, VisualConfig::ColorNodeHeader ) );
	EXPECT_TRUE( regionContainsColor( image, 260U, 140U, 270U, 150U, VisualConfig::ColorNodeBody ) );
}

TEST( GraphRendererTests, DrawsForwardEdgeBetweenNodes ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "From", .x = 50.0f, .y = 100.0f } );
	graph.addNode( { .id = 2, .name = "To", .x = 400.0f, .y = 100.0f } );
	ASSERT_TRUE( graph.addEdge( { .from = 1, .to = 2 } ) );

	const sf::Image image = renderGraphToImage( graph );

	EXPECT_TRUE( regionContainsColor( image, 296U, 136U, 304U, 144U, VisualConfig::ColorEdge ) );
}

TEST( GraphRendererTests, DrawsBackwardEdgeWithDetourAboveNodes ) {
	Graph graph;
	graph.addNode( { .id = 1, .name = "From", .x = 420.0f, .y = 170.0f } );
	graph.addNode( { .id = 2, .name = "To", .x = 60.0f, .y = 110.0f } );
	ASSERT_TRUE( graph.addEdge( { .from = 1, .to = 2 } ) );

	const sf::Image image = renderGraphToImage( graph );

	EXPECT_TRUE( regionContainsColor( image, 296U, 66U, 304U, 74U, VisualConfig::ColorEdge ) );
}

TEST( GraphRendererTests, DrawsSelfLoopWithDetourAboveNode ) {
	Graph graph;
	graph.addNode( { .id = 7, .name = "Loop", .x = 220.0f, .y = 160.0f } );
	ASSERT_TRUE( graph.addEdge( { .from = 7, .to = 7 } ) );

	const sf::Image image = renderGraphToImage( graph );

	EXPECT_TRUE( regionContainsColor( image, 296U, 116U, 304U, 124U, VisualConfig::ColorEdge ) );
}

}  // namespace task2
