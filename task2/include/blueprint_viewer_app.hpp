#pragma once

#include "blueprint_file_manager.hpp"
#include "blueprint_viewer_panel.hpp"
#include "graph.hpp"
#include "graph_view_controller.hpp"
#include "imgui_sfml_context.hpp"
#include "layout_engine.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <array>
#include <string>
#include <string_view>

namespace task2 {

class BlueprintViewerApp {
public:
	BlueprintViewerApp();
	~BlueprintViewerApp() noexcept = default;

	BlueprintViewerApp( const BlueprintViewerApp& )            = delete;
	BlueprintViewerApp& operator=( const BlueprintViewerApp& ) = delete;
	BlueprintViewerApp( BlueprintViewerApp&& )                 = delete;
	BlueprintViewerApp& operator=( BlueprintViewerApp&& )      = delete;

	[[nodiscard]] int run();

private:
	void initializeWindow();
	void loadFont();
	void initializeOutputFileNameBuffer();

	void processEvents();
	void handleEvent( const sf::Event& event );
	void handleSystemEvent( const sf::Event& event );
	void handleKeyboardEvent( const sf::Event::KeyPressed& event );
	void handleMouseWheelEvent( const sf::Event::MouseWheelScrolled& event );
	void handleMouseButtonEvent( const sf::Event& event );
	void handleMouseMoveEvent( const sf::Event::MouseMoved& event );

	void update( sf::Time deltaTime );
	void render();

	[[nodiscard]] BlueprintViewerPanel::State makePanelState();
	void handlePanelResult( const BlueprintViewerPanel::Result& result );

	void loadGraph();
	void saveGraph();
	void applyLayout();
	void refreshInputFiles();
	void syncOutputFileName();

	[[nodiscard]] std::string outputFileNameFromBuffer() const;
	void setOutputFileNameBuffer( std::string_view fileName );

	static constexpr unsigned int FrameRateLimit = 60;
	static constexpr float KeyboardZoomFactor    = 1.1f;
	static constexpr float ZoomInFactor          = 0.9f;
	static constexpr float ZoomOutFactor         = 1.1f;

	static constexpr std::size_t OutputFileNameBufferSize = BlueprintViewerPanel::OutputFileNameBufferSize;

	sf::RenderWindow window_;
	GraphViewController viewController_;
	ImGuiSfmlContext imguiContext_;
	BlueprintFileManager fileManager_;
	BlueprintViewerPanel panel_;
	LayoutEngine::Config layoutConfig_;

	sf::Font font_;
	sf::Clock deltaClock_;
	Graph graph_;
	std::array< char, OutputFileNameBufferSize > outputFileNameBuffer_{};
	std::string statusMessage_{ "Ready" };
};

}  // namespace task2
