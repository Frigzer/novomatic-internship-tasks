#pragma once

#include "graph.hpp"
#include "graph_renderer.hpp"
#include "layout_engine.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <array>
#include <filesystem>
#include <string>

namespace task2 {

class BlueprintViewerApp {
public:
	BlueprintViewerApp();
	~BlueprintViewerApp();

	int run();

private:
	void processEvents();
	void handleEvent( const sf::Event& event );
	void update( sf::Time deltaTime );
	void render();

	void drawGui();

	void loadGraph();
	void saveGraph();
	void applyLayout();

	void resetView();
	void fitGraphInView();

	sf::RenderWindow window_;
	sf::View graphView_;
	sf::Font font_;
	sf::Clock deltaClock_;

	Graph graph_;
	GraphRenderer renderer_;
	LayoutEngine::Config layoutConfig_;

	bool isPanning_{ false };
	sf::Vector2i lastMousePixel_{};

	std::filesystem::path inputPath_;
	std::filesystem::path outputPath_;
	std::filesystem::path fontPath_;

	std::array< char, 512 > inputPathBuffer_{};
	std::array< char, 512 > outputPathBuffer_{};

	std::string statusMessage_{ "Ready" };
};

}  // namespace task2