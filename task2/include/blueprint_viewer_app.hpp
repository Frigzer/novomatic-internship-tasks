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
#include <vector>

namespace task2 {

class BlueprintViewerApp {
public:
	BlueprintViewerApp();
	~BlueprintViewerApp();

	int run();

private:
	struct GuiLimits {
		static constexpr float MarginMin = 0.0f;
		static constexpr float MarginMax = 500.0f;

		static constexpr float LayerSpacingMin = 50.0f;
		static constexpr float LayerSpacingMax = 600.0f;

		static constexpr float NodeSpacingMin = 20.0f;
		static constexpr float NodeSpacingMax = 300.0f;

		static constexpr float ComponentSpacingMin = 50.0f;
		static constexpr float ComponentSpacingMax = 500.0f;

		static constexpr float DefaultWidth  = 400.0f;
		static constexpr float DefaultHeight = 550.0f;
	};

	static constexpr std::size_t FileNameBufferSize = 256;

	void processEvents();
	void handleEvent( const sf::Event& event );

	void handleSystemEvent( const sf::Event& event );
    void handleKeyboardEvent( const sf::Event::KeyPressed& event );
    void handleMouseWheelEvent( const sf::Event::MouseWheelScrolled& event );
    void handleMouseButtonEvent( const sf::Event& event );
    void handleMouseMoveEvent( const sf::Event::MouseMoved& event );

	void update( sf::Time deltaTime );
	void render();

	void drawGui();

	void loadGraph();
	void saveGraph();
	void applyLayout();

	void resetView();
	void fitGraphInView();

	void refreshInputFiles();

	const Node* findHoveredNode() const;

	sf::RenderWindow window_;
	sf::View graphView_;
	sf::Font font_;
	sf::Clock deltaClock_;

	Graph graph_;
	GraphRenderer renderer_;
	LayoutEngine::Config layoutConfig_;

	bool isPanning_{ false };
	sf::Vector2i lastMousePixel_;

	std::filesystem::path inputPath_;
	std::filesystem::path outputPath_;
	std::filesystem::path fontPath_;

	std::vector< std::filesystem::path > inputFiles_;
	int selectedInputIndex_{ -1 };

	std::array< char, FileNameBufferSize > outputFileNameBuffer_{};

	std::string statusMessage_{ "Ready" };
};

}  // namespace task2