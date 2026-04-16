#include "blueprint_viewer_app.hpp"

#include "graph_renderer.hpp"
#include "json_io.hpp"
#include "paths.hpp"
#include "visual_config.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <exception>
#include <optional>
#include <stdexcept>

namespace task2 {

namespace {

// Window configuration
constexpr unsigned int DefaultWindowWidth  = 1600;
constexpr unsigned int DefaultWindowHeight = 900;
constexpr unsigned int FrameRateLimit      = 60;
const std::string WindowTitle              = "Task 2 - Blueprint Auto Layout";

// Default filenames
const std::string DefaultInputFile  = "sample_graph.json";
const std::string DefaultOutputFile = "sample_graph_out.json";
const std::string MainFontFile      = "font/arial.ttf";
}  // namespace

BlueprintViewerApp::BlueprintViewerApp()
    : window_( sf::VideoMode( { DefaultWindowWidth, DefaultWindowHeight } ), WindowTitle ),
      graphView_( window_.getDefaultView() ),
      inputPath_( paths::inputDir / DefaultInputFile ),
      outputPath_( paths::outputDir / DefaultOutputFile ),
      fontPath_( paths::dataDir / MainFontFile ) {
	window_.setFramerateLimit( FrameRateLimit );

	if ( !ImGui::SFML::Init( window_ ) ) {
		throw std::runtime_error( "Failed to initialize ImGui-SFML" );
	}

	if ( !font_.openFromFile( fontPath_.string() ) ) {
		throw std::runtime_error( "Failed to load font: " + fontPath_.string() );
	}

	std::filesystem::create_directories( paths::inputDir );
	std::filesystem::create_directories( paths::outputDir );

	std::snprintf( outputFileNameBuffer_.data(), outputFileNameBuffer_.size(), "%s",
	               outputPath_.filename().string().c_str() );

	refreshInputFiles();

	if ( !inputFiles_.empty() ) {
		auto it = std::ranges::find( inputFiles_, inputPath_ );
		if ( it != inputFiles_.end() ) {
			selectedInputIndex_ = static_cast< int >( std::distance( inputFiles_.begin(), it ) );
		} else {
			selectedInputIndex_ = 0;
			inputPath_          = inputFiles_.front();
		}
	}
	loadGraph();
	resetView();
}

BlueprintViewerApp::~BlueprintViewerApp() {
	ImGui::SFML::Shutdown();
}

void BlueprintViewerApp::refreshInputFiles() {
	inputFiles_.clear();

	if ( !std::filesystem::exists( paths::inputDir ) ) {
		return;
	}

	for ( const auto& entry : std::filesystem::directory_iterator( paths::inputDir ) ) {
		if ( !entry.is_regular_file() ) {
			continue;
		}

		if ( entry.path().extension() == ".json" ) {
			inputFiles_.push_back( entry.path() );
		}
	}

	std::ranges::sort( inputFiles_,
	                   []( const auto& a, const auto& b ) { return a.filename().string() < b.filename().string(); } );

	if ( inputFiles_.empty() ) {
		selectedInputIndex_ = -1;
	} else if ( selectedInputIndex_ >= static_cast< int >( inputFiles_.size() ) ) {
		selectedInputIndex_ = 0;
	}
}

const Node* BlueprintViewerApp::findHoveredNode() const {
	const sf::Vector2i mousePixel = sf::Mouse::getPosition( window_ );
	const sf::Vector2f mouseWorld = window_.mapPixelToCoords( mousePixel, graphView_ );

	for ( const auto& [ _, node ] : graph_.getNodes() ) {
		const sf::FloatRect bounds( { node.x, node.y }, { node.width, node.height } );

		if ( bounds.contains( mouseWorld ) ) {
			return &node;
		}
	}

	return nullptr;
}

int BlueprintViewerApp::run() {
	while ( window_.isOpen() ) {
		processEvents();
		update( deltaClock_.restart() );
		render();
	}

	return 0;
}

void BlueprintViewerApp::processEvents() {
	while ( const std::optional event = window_.pollEvent() ) {
		ImGui::SFML::ProcessEvent( window_, *event );
		handleEvent( *event );
	}
}

void BlueprintViewerApp::handleEvent( const sf::Event& event ) {
	handleSystemEvent( event );

	if ( const auto* keyPressed = event.getIf< sf::Event::KeyPressed >() ) {
		handleKeyboardEvent( *keyPressed );
	} else if ( const auto* mouseWheel = event.getIf< sf::Event::MouseWheelScrolled >() ) {
		handleMouseWheelEvent( *mouseWheel );
	} else if ( event.is< sf::Event::MouseButtonPressed >() || event.is< sf::Event::MouseButtonReleased >() ) {
		handleMouseButtonEvent( event );
	} else if ( const auto* mouseMoved = event.getIf< sf::Event::MouseMoved >() ) {
		handleMouseMoveEvent( *mouseMoved );
	}
}

void BlueprintViewerApp::handleSystemEvent( const sf::Event& event ) {
	if ( event.is< sf::Event::Closed >() ) {
		window_.close();
	} else if ( const auto* resized = event.getIf< sf::Event::Resized >() ) {
		graphView_.setSize( { static_cast< float >( resized->size.x ), static_cast< float >( resized->size.y ) } );
	}
}

void BlueprintViewerApp::handleKeyboardEvent( const sf::Event::KeyPressed& event ) {
	if ( ImGui::GetIO().WantCaptureKeyboard ) return;

	const float keyboardZoomFactor = 1.1f;

	if ( event.code == sf::Keyboard::Key::Add || event.code == sf::Keyboard::Key::Equal ) {
		graphView_.zoom( 1.0f / keyboardZoomFactor );
	} else if ( event.code == sf::Keyboard::Key::Subtract || event.code == sf::Keyboard::Key::Hyphen ) {
		graphView_.zoom( keyboardZoomFactor );
	}
}

void BlueprintViewerApp::handleMouseWheelEvent( const sf::Event::MouseWheelScrolled& event ) {
	if ( ImGui::GetIO().WantCaptureMouse || event.wheel != sf::Mouse::Wheel::Vertical ) return;

	const float zoomFactor        = event.delta > 0 ? 0.9f : 1.1f;
	const sf::Vector2i mousePixel = sf::Mouse::getPosition( window_ );
	const sf::Vector2f beforeZoom = window_.mapPixelToCoords( mousePixel, graphView_ );

	graphView_.zoom( zoomFactor );

	const sf::Vector2f afterZoom = window_.mapPixelToCoords( mousePixel, graphView_ );
	graphView_.move( beforeZoom - afterZoom );
}

void BlueprintViewerApp::handleMouseButtonEvent( const sf::Event& event ) {
	if ( ImGui::GetIO().WantCaptureMouse ) return;

	if ( const auto* pressed = event.getIf< sf::Event::MouseButtonPressed >() ) {
		const bool isMiddle = ( pressed->button == sf::Mouse::Button::Middle );
		const bool isCtrlLeft =
		    ( pressed->button == sf::Mouse::Button::Left && sf::Keyboard::isKeyPressed( sf::Keyboard::Key::LControl ) );

		if ( isMiddle || isCtrlLeft ) {
			isPanning_      = true;
			lastMousePixel_ = { pressed->position.x, pressed->position.y };
		}
	} else if ( const auto* released = event.getIf< sf::Event::MouseButtonReleased >() ) {
		if ( released->button == sf::Mouse::Button::Middle || released->button == sf::Mouse::Button::Left ) {
			isPanning_ = false;
		}
	}
}

void BlueprintViewerApp::handleMouseMoveEvent( const sf::Event::MouseMoved& event ) {
	if ( !isPanning_ ) return;

	const sf::Vector2i currentPixel{ event.position.x, event.position.y };
	const sf::Vector2f previousWorld = window_.mapPixelToCoords( lastMousePixel_, graphView_ );
	const sf::Vector2f currentWorld  = window_.mapPixelToCoords( currentPixel, graphView_ );

	graphView_.move( previousWorld - currentWorld );
	lastMousePixel_ = currentPixel;
}

void BlueprintViewerApp::update( sf::Time deltaTime ) {
	ImGui::SFML::Update( window_, deltaTime );
	drawGui();
}

void BlueprintViewerApp::render() {
	window_.clear( VisualConfig::ColorBackground );

	window_.setView( graphView_ );
	GraphRenderer::draw( window_, graph_, font_ );

	window_.setView( window_.getDefaultView() );
	ImGui::SFML::Render( window_ );

	window_.display();
}

void BlueprintViewerApp::drawGui() {
	using GL = GuiLimits;

	ImGui::SetNextWindowSize( { GL::DefaultWidth, GL::DefaultHeight }, ImGuiCond_FirstUseEver );

	ImGui::Begin( "Controls" );

	if ( ImGui::Button( "Refresh input files" ) ) {
		refreshInputFiles();
	}

	if ( !inputFiles_.empty() && selectedInputIndex_ >= 0 &&
	     selectedInputIndex_ < static_cast< int >( inputFiles_.size() ) ) {
		const std::string currentLabel = inputFiles_[ selectedInputIndex_ ].filename().string();

		if ( ImGui::BeginCombo( "Input graph", currentLabel.c_str() ) ) {
			for ( int i = 0; i < static_cast< int >( inputFiles_.size() ); ++i ) {
				const bool isSelected   = ( i == selectedInputIndex_ );
				const std::string label = inputFiles_[ i ].filename().string();

				if ( ImGui::Selectable( label.c_str(), isSelected ) ) {
					selectedInputIndex_ = i;
					inputPath_          = inputFiles_[ i ];
				}

				if ( isSelected ) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	} else {
		ImGui::TextDisabled( "No input JSON files found in data/input" );
	}

	ImGui::InputText( "Output file name", outputFileNameBuffer_.data(), outputFileNameBuffer_.size() );

	ImGui::Separator();

	ImGui::SliderFloat( "Margin X", &layoutConfig_.margin_x, GL::MarginMin, GL::MarginMax );
	ImGui::SliderFloat( "Margin Y", &layoutConfig_.margin_y, GL::MarginMin, GL::MarginMax );

	ImGui::SliderFloat( "Layer spacing", &layoutConfig_.layer_spacing, GL::LayerSpacingMin, GL::LayerSpacingMax );

	ImGui::SliderFloat( "Node spacing", &layoutConfig_.node_spacing, GL::NodeSpacingMin, GL::NodeSpacingMax );

	ImGui::SliderFloat( "Component spacing", &layoutConfig_.component_spacing, GL::ComponentSpacingMin,
	                    GL::ComponentSpacingMax );

	if ( ImGui::Button( "Load JSON" ) ) {
		loadGraph();
	}

	ImGui::SameLine();

	if ( ImGui::Button( "Apply Layout" ) ) {
		applyLayout();
	}

	ImGui::SameLine();

	if ( ImGui::Button( "Save JSON" ) ) {
		saveGraph();
	}

	if ( ImGui::Button( "Reset View" ) ) {
		resetView();
	}

	ImGui::SameLine();

	if ( ImGui::Button( "Fit Graph" ) ) {
		fitGraphInView();
	}

	ImGui::Separator();

	ImGui::Text( "Nodes: %zu", graph_.getNodes().size() );
	ImGui::Text( "Edges: %zu", graph_.getEdges().size() );

	ImGui::TextWrapped( "Status: %s", statusMessage_.c_str() );

	ImGui::Separator();

	auto getCleanPath = []( const std::filesystem::path& path ) {
		return path.parent_path().filename().string() + "/" + path.filename().string();
	};

	ImGui::TextDisabled( "Input:  %s", getCleanPath( paths::inputDir ).c_str() );
	ImGui::TextDisabled( "Output: %s", getCleanPath( paths::outputDir ).c_str() );

	ImGui::Separator();

	ImGui::Text( "Controls:" );
	ImGui::BulletText( "Zoom:  Wheel or Keypad +/-" );
	ImGui::BulletText( "Pan:   MMB or Ctrl + LPM" );
	ImGui::BulletText( "Reset: 'Reset View' or 'Fit Graph'" );

	if ( !ImGui::GetIO().WantCaptureMouse ) {
		if ( const Node* hoveredNode = findHoveredNode(); hoveredNode != nullptr ) {
			ImGui::BeginTooltip();
			ImGui::Text( "%s", hoveredNode->name.c_str() );
			ImGui::TextDisabled( "Node ID: %d", hoveredNode->id );
			ImGui::EndTooltip();
		}
	}

	ImGui::End();
}

void BlueprintViewerApp::loadGraph() {
	try {
		if ( selectedInputIndex_ < 0 || selectedInputIndex_ >= static_cast< int >( inputFiles_.size() ) ) {
			statusMessage_ = "Load failed: no input file selected";
			return;
		}

		inputPath_     = inputFiles_[ selectedInputIndex_ ];
		graph_         = JsonGraphIO::loadFromFile( inputPath_ );
		statusMessage_ = "Loaded graph successfully: " + inputPath_.filename().string();
		// fitGraphInView();
	} catch ( const std::exception& ex ) {
		statusMessage_ = std::string( "Load failed: " ) + ex.what();
	}
}

void BlueprintViewerApp::saveGraph() {
	try {
		const std::string outputFileName = outputFileNameBuffer_.data();

		if ( outputFileName.empty() ) {
			statusMessage_ = "Save failed: output file name is empty";
			return;
		}

		outputPath_ = paths::outputDir / outputFileName;
		JsonGraphIO::saveToFile( graph_, outputPath_ );
		statusMessage_ = "Saved graph successfully: " + outputPath_.filename().string();
	} catch ( const std::exception& ex ) {
		statusMessage_ = std::string( "Save failed: " ) + ex.what();
	}
}

void BlueprintViewerApp::applyLayout() {
	try {
		LayoutEngine engine( layoutConfig_ );
		engine.applyLayout( graph_ );
		statusMessage_ = "Layout applied successfully";
		// fitGraphInView();
	} catch ( const std::exception& ex ) {
		statusMessage_ = std::string( "Layout failed: " ) + ex.what();
	}
}

void BlueprintViewerApp::resetView() {
	graphView_ = window_.getDefaultView();
}

void BlueprintViewerApp::fitGraphInView() {
	if ( graph_.getNodes().empty() ) {
		resetView();
		return;
	}

	float minX = std::numeric_limits< float >::max();
	float minY = std::numeric_limits< float >::max();
	float maxX = std::numeric_limits< float >::lowest();
	float maxY = std::numeric_limits< float >::lowest();

	for ( const auto& [ _, node ] : graph_.getNodes() ) {
		minX = std::min( minX, node.x );
		minY = std::min( minY, node.y );
		maxX = std::max( maxX, node.x + node.width );
		maxY = std::max( maxY, node.y + node.height );
	}

	const float padding = 80.0f;
	minX -= padding;
	minY -= padding;
	maxX += padding;
	maxY += padding;

	const float graphWidth  = std::max( 1.0f, maxX - minX );
	const float graphHeight = std::max( 1.0f, maxY - minY );

	graphView_.setCenter(
	    { minX + ( graphWidth * 0.5f ),  // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
	      minY +
	          ( graphHeight * 0.5f ) } );  // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
	graphView_.setSize( { graphWidth, graphHeight } );

	const sf::Vector2u windowSize = window_.getSize();
	if ( windowSize.x == 0 || windowSize.y == 0 ) {
		return;
	}

	const float windowAspect = static_cast< float >( windowSize.x ) / static_cast< float >( windowSize.y );
	const float graphAspect  = graphWidth / graphHeight;

	if ( graphAspect > windowAspect ) {
		graphView_.setSize( { graphWidth, graphWidth / windowAspect } );
	} else {
		graphView_.setSize( { graphHeight * windowAspect, graphHeight } );
	}
}

}  // namespace task2