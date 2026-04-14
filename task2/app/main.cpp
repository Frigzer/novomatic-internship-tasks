#include "graph_renderer.hpp"
#include "json_io.hpp"
#include "layout_engine.hpp"
#include "paths.hpp"

#include <imgui-SFML.h>
#include <imgui.h>


#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <array>
#include <exception>
#include <iostream>
#include <string>

int main() {
	try {
		const auto defaultInputPath  = task2::paths::dataDir / "sample_graph.json";
		const auto defaultOutputPath = task2::paths::dataDir / "sample_graph_out.json";
		const auto fontPath          = task2::paths::dataDir / "arial.ttf";

		sf::RenderWindow window( sf::VideoMode( { 1600, 900 } ), "Task 2 - Blueprint Auto Layout" );
		window.setFramerateLimit( 60 );

		if ( !ImGui::SFML::Init( window ) ) {
			std::cerr << "Failed to initialize ImGui-SFML\n";
			return 1;
		}

		sf::Font font;
		if ( !font.openFromFile( fontPath.string() ) ) {
			std::cerr << "Failed to load font: " << fontPath << '\n';
			return 1;
		}

		task2::Graph graph = task2::JsonGraphIO::loadFromFile( defaultInputPath );
		task2::LayoutEngine::Config layoutConfig;
		task2::LayoutEngine layoutEngine( layoutConfig );
		task2::GraphRenderer renderer;

		std::array< char, 512 > inputPathBuffer{};
		std::array< char, 512 > outputPathBuffer{};
		std::snprintf( inputPathBuffer.data(), inputPathBuffer.size(), "%s", defaultInputPath.string().c_str() );
		std::snprintf( outputPathBuffer.data(), outputPathBuffer.size(), "%s", defaultOutputPath.string().c_str() );

		std::string statusMessage = "Ready";
		sf::Clock deltaClock;

		while ( window.isOpen() ) {
			while ( const std::optional event = window.pollEvent() ) {
				ImGui::SFML::ProcessEvent( window, *event );

				if ( event->is< sf::Event::Closed >() ) {
					window.close();
				}
			}

			ImGui::SFML::Update( window, deltaClock.restart() );

			ImGui::Begin( "Controls" );

			ImGui::InputText( "Input JSON", inputPathBuffer.data(), inputPathBuffer.size() );
			ImGui::InputText( "Output JSON", outputPathBuffer.data(), outputPathBuffer.size() );

			ImGui::Separator();

			ImGui::SliderFloat( "Margin X", &layoutConfig.margin_x, 0.0f, 500.0f );
			ImGui::SliderFloat( "Margin Y", &layoutConfig.margin_y, 0.0f, 500.0f );
			ImGui::SliderFloat( "Layer spacing", &layoutConfig.layer_spacing, 50.0f, 600.0f );
			ImGui::SliderFloat( "Node spacing", &layoutConfig.node_spacing, 20.0f, 300.0f );
			ImGui::SliderFloat( "Component spacing", &layoutConfig.component_spacing, 50.0f, 400.0f );

			if ( ImGui::Button( "Load JSON" ) ) {
				try {
					graph         = task2::JsonGraphIO::loadFromFile( inputPathBuffer.data() );
					statusMessage = "Loaded graph successfully";
				} catch ( const std::exception& ex ) {
					statusMessage = std::string( "Load failed: " ) + ex.what();
				}
			}

			ImGui::SameLine();

			if ( ImGui::Button( "Apply Layout" ) ) {
				try {
					layoutEngine = task2::LayoutEngine( layoutConfig );
					layoutEngine.applyLayout( graph );
					statusMessage = "Layout applied successfully";
				} catch ( const std::exception& ex ) {
					statusMessage = std::string( "Layout failed: " ) + ex.what();
				}
			}

			ImGui::SameLine();

			if ( ImGui::Button( "Save JSON" ) ) {
				try {
					task2::JsonGraphIO::saveToFile( graph, outputPathBuffer.data() );
					statusMessage = "Saved graph successfully";
				} catch ( const std::exception& ex ) {
					statusMessage = std::string( "Save failed: " ) + ex.what();
				}
			}

			ImGui::Separator();
			ImGui::TextWrapped( "Status: %s", statusMessage.c_str() );
			ImGui::Text( "Nodes: %zu", graph.getNodes().size() );
			ImGui::Text( "Edges: %zu", graph.getEdges().size() );

			ImGui::End();

			window.clear( sf::Color( 24, 26, 30 ) );
			renderer.draw( window, graph, font );
			ImGui::SFML::Render( window );
			window.display();
		}

		ImGui::SFML::Shutdown();
	} catch ( const std::exception& ex ) {
		std::cerr << "Fatal error: " << ex.what() << '\n';
		return 1;
	}

	return 0;
}