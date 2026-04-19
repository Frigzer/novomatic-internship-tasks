#include "blueprint_viewer_app.hpp"

#include "graph_renderer.hpp"
#include "paths.hpp"
#include "visual_config.hpp"

#include <imgui.h>

#include <algorithm>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace task2 {

namespace {

constexpr sf::Vector2u DefaultWindowSize{ 1600U, 900U };

const std::string WindowTitle       = "Task 2 - Blueprint Auto Layout";
const std::string DefaultInputFile  = "sample_graph.json";
const std::string DefaultOutputFile = "sample_graph_out.json";
const std::string MainFontFile      = "font/arial.ttf";

}  // namespace

BlueprintViewerApp::BlueprintViewerApp()
    : window_( sf::VideoMode( DefaultWindowSize ), WindowTitle ),
      viewController_( window_.getDefaultView() ),
      imguiContext_( window_ ),
      fileManager_( paths::inputDir, paths::outputDir, paths::inputDir / DefaultInputFile,
                    paths::outputDir / DefaultOutputFile ) {
	initializeWindow();
	loadFont();
	fileManager_.ensureDirectoriesExist();
	refreshInputFiles();
	if ( fileManager_.hasSelectedInputFile() ) {
		loadGraph();
	}
	initializeOutputFileNameBuffer();
	viewController_.resetToDefault( window_ );
}

int BlueprintViewerApp::run() {
	while ( window_.isOpen() ) {
		processEvents();
		update( deltaClock_.restart() );
		render();
	}

	return 0;
}

void BlueprintViewerApp::initializeWindow() {
	window_.setFramerateLimit( FrameRateLimit );
}

void BlueprintViewerApp::loadFont() {
	const std::filesystem::path fontPath = paths::dataDir / MainFontFile;
	if ( !font_.openFromFile( fontPath.string() ) ) {
		throw std::runtime_error( "Failed to load font: " + fontPath.string() );
	}
}

void BlueprintViewerApp::initializeOutputFileNameBuffer() {
	setOutputFileNameBuffer( fileManager_.outputFileName() );
}

void BlueprintViewerApp::processEvents() {
	while ( const std::optional event = window_.pollEvent() ) {
		ImGuiSfmlContext::processEvent( window_, *event );
		handleEvent( *event );
	}
}

void BlueprintViewerApp::handleEvent( const sf::Event& event ) {
	handleSystemEvent( event );

	if ( const auto* keyPressed = event.getIf< sf::Event::KeyPressed >() ) {
		handleKeyboardEvent( *keyPressed );
		return;
	}

	if ( const auto* mouseWheel = event.getIf< sf::Event::MouseWheelScrolled >() ) {
		handleMouseWheelEvent( *mouseWheel );
		return;
	}

	if ( event.is< sf::Event::MouseButtonPressed >() || event.is< sf::Event::MouseButtonReleased >() ) {
		handleMouseButtonEvent( event );
		return;
	}

	if ( const auto* mouseMoved = event.getIf< sf::Event::MouseMoved >() ) {
		handleMouseMoveEvent( *mouseMoved );
	}
}

void BlueprintViewerApp::handleSystemEvent( const sf::Event& event ) {
	if ( event.is< sf::Event::Closed >() ) {
		window_.close();
		return;
	}

	if ( const auto* resized = event.getIf< sf::Event::Resized >() ) {
		viewController_.resizeToWindow( resized->size );
	}
}

void BlueprintViewerApp::handleKeyboardEvent( const sf::Event::KeyPressed& event ) {
	if ( ImGui::GetIO().WantCaptureKeyboard ) {
		return;
	}

	const sf::Vector2u windowSize = window_.getSize();
	const sf::Vector2i viewCenterPixel{ static_cast< int >( windowSize.x / 2U ),
	                                    static_cast< int >( windowSize.y / 2U ) };

	if ( event.code == sf::Keyboard::Key::Add || event.code == sf::Keyboard::Key::Equal ) {
		viewController_.zoomAtPixel( window_, 1.0f / KeyboardZoomFactor, viewCenterPixel );
	} else if ( event.code == sf::Keyboard::Key::Subtract || event.code == sf::Keyboard::Key::Hyphen ) {
		viewController_.zoomAtPixel( window_, KeyboardZoomFactor, viewCenterPixel );
	}
}

void BlueprintViewerApp::handleMouseWheelEvent( const sf::Event::MouseWheelScrolled& event ) {
	if ( ImGui::GetIO().WantCaptureMouse || event.wheel != sf::Mouse::Wheel::Vertical ) {
		return;
	}

	const float zoomFactor = ( event.delta > 0.0f ) ? ZoomInFactor : ZoomOutFactor;
	viewController_.zoomAtPixel( window_, zoomFactor, sf::Mouse::getPosition( window_ ) );
}

void BlueprintViewerApp::handleMouseButtonEvent( const sf::Event& event ) {
	if ( ImGui::GetIO().WantCaptureMouse ) {
		return;
	}

	if ( const auto* pressed = event.getIf< sf::Event::MouseButtonPressed >() ) {
		const bool isMiddleButton = pressed->button == sf::Mouse::Button::Middle;
		const bool isCtrlLeftButton =
		    pressed->button == sf::Mouse::Button::Left && sf::Keyboard::isKeyPressed( sf::Keyboard::Key::LControl );

		if ( isMiddleButton || isCtrlLeftButton ) {
			viewController_.beginPanning( { pressed->position.x, pressed->position.y } );
		}

		return;
	}

	if ( const auto* released = event.getIf< sf::Event::MouseButtonReleased >() ) {
		if ( released->button == sf::Mouse::Button::Middle || released->button == sf::Mouse::Button::Left ) {
			viewController_.endPanning();
		}
	}
}

void BlueprintViewerApp::handleMouseMoveEvent( const sf::Event::MouseMoved& event ) {
	viewController_.panTo( window_, { event.position.x, event.position.y } );
}

void BlueprintViewerApp::update( sf::Time deltaTime ) {
	ImGuiSfmlContext::update( window_, deltaTime );
	const auto panelResult = BlueprintViewerPanel::draw( makePanelState() );
	handlePanelResult( panelResult );
}

void BlueprintViewerApp::render() {
	window_.clear( VisualConfig::ColorBackground );
	window_.setView( viewController_.view() );
	GraphRenderer::draw( window_, graph_, font_ );
	window_.setView( window_.getDefaultView() );
	ImGuiSfmlContext::render( window_ );
	window_.display();
}

BlueprintViewerPanel::State BlueprintViewerApp::makePanelState() {
	std::optional< BlueprintViewerPanel::HoveredNodeInfo > hoveredNode;
	if ( !ImGui::GetIO().WantCaptureMouse ) {
		if ( const Node* node = viewController_.findHoveredNode( window_, graph_ ); node != nullptr ) {
			hoveredNode = BlueprintViewerPanel::HoveredNodeInfo{ .id = node->id, .name = node->name };
		}
	}

	return BlueprintViewerPanel::State{ .inputFiles           = fileManager_.inputFiles(),
	                                    .selectedInputIndex   = fileManager_.selectedInputIndex(),
	                                    .outputFileNameBuffer = outputFileNameBuffer_,
	                                    .layoutConfig         = layoutConfig_,
	                                    .statusMessage        = statusMessage_,
	                                    .nodeCount            = graph_.getNodes().size(),
	                                    .edgeCount            = graph_.getEdges().size(),
	                                    .inputDirectory       = paths::inputDir,
	                                    .outputDirectory      = paths::outputDir,
	                                    .hoveredNode          = std::move( hoveredNode ) };
}

void BlueprintViewerApp::handlePanelResult( const BlueprintViewerPanel::Result& result ) {
	if ( result.selectedInputIndex.has_value() ) {
		fileManager_.setSelectedInputIndex( *result.selectedInputIndex );
	}

	if ( result.refreshInputFilesRequested ) {
		refreshInputFiles();
	}

	if ( result.loadGraphRequested ) {
		loadGraph();
	}

	if ( result.applyLayoutRequested ) {
		applyLayout();
	}

	if ( result.saveGraphRequested ) {
		saveGraph();
	}

	if ( result.resetViewRequested ) {
		viewController_.resetToDefault( window_ );
	}

	if ( result.fitGraphRequested ) {
		viewController_.fitGraph( window_, graph_ );
	}
}

void BlueprintViewerApp::loadGraph() {
	if ( !fileManager_.hasSelectedInputFile() ) {
		statusMessage_ = "Load failed: no input file selected";
		return;
	}

	try {
		graph_         = fileManager_.loadSelectedGraph();
		statusMessage_ = "Loaded graph successfully: " + fileManager_.selectedInputLabel();
	} catch ( const std::exception& ex ) {
		statusMessage_ = std::string( "Load failed: " ) + ex.what();
	}
}

void BlueprintViewerApp::saveGraph() {
	try {
		syncOutputFileName();
		fileManager_.saveGraph( graph_ );
		statusMessage_ = "Saved graph successfully: " + fileManager_.outputPath().filename().string();
	} catch ( const std::exception& ex ) {
		statusMessage_ = std::string( "Save failed: " ) + ex.what();
	}
}

void BlueprintViewerApp::applyLayout() {
	try {
		LayoutEngine engine( layoutConfig_ );
		engine.applyLayout( graph_ );
		statusMessage_ = "Layout applied successfully";
	} catch ( const std::exception& ex ) {
		statusMessage_ = std::string( "Layout failed: " ) + ex.what();
	}
}

void BlueprintViewerApp::refreshInputFiles() {
	fileManager_.refreshInputFiles();
	if ( fileManager_.hasSelectedInputFile() ) {
		statusMessage_ = "Input file list refreshed";
		return;
	}

	statusMessage_ = "No input JSON files found in data/input";
}

void BlueprintViewerApp::syncOutputFileName() {
	fileManager_.setOutputFileName( outputFileNameFromBuffer() );
	setOutputFileNameBuffer( fileManager_.outputFileName() );
}

std::string BlueprintViewerApp::outputFileNameFromBuffer() const {
	return outputFileNameBuffer_.data();
}

void BlueprintViewerApp::setOutputFileNameBuffer( std::string_view fileName ) {
	outputFileNameBuffer_.fill( '\0' );
	const std::size_t maxCopyLength = outputFileNameBuffer_.size() - 1U;
	const std::size_t copyLength    = std::min( fileName.size(), maxCopyLength );
	std::ranges::copy( fileName.substr( 0, copyLength ), outputFileNameBuffer_.begin() );
}

}  // namespace task2
