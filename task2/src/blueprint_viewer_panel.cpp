#include "blueprint_viewer_panel.hpp"

#include <imgui.h>

namespace task2 {

namespace {

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

[[nodiscard]] bool isValidSelection( const BlueprintViewerPanel::State& state ) {
	return state.selectedInputIndex >= 0 && state.selectedInputIndex < static_cast< int >( state.inputFiles.size() );
}

bool hasSelectedInputFile( const BlueprintViewerPanel::State& state ) {
	return isValidSelection( state );
}

std::string makeCompactPathLabel( const std::filesystem::path& path ) {
	return path.parent_path().filename().string() + "/" + path.filename().string();
}

std::string selectedInputLabel( const BlueprintViewerPanel::State& state ) {
	if ( !hasSelectedInputFile( state ) ) {
		return "<none>";
	}

	return state.inputFiles[ state.selectedInputIndex ].filename().string();
}

void drawInputSection( BlueprintViewerPanel::State state, BlueprintViewerPanel::Result& result ) {
	if ( ImGui::Button( "Refresh input files" ) ) {
		result.refreshInputFilesRequested = true;
	}

	if ( hasSelectedInputFile( state ) ) {
		const std::string currentLabel = selectedInputLabel( state );

		if ( ImGui::BeginCombo( "Input graph", currentLabel.c_str() ) ) {
			for ( int index = 0; index < static_cast< int >( state.inputFiles.size() ); ++index ) {
				const bool isSelected   = index == state.selectedInputIndex;
				const std::string label = state.inputFiles[ index ].filename().string();

				if ( ImGui::Selectable( label.c_str(), isSelected ) ) {
					result.selectedInputIndex = index;
					state.selectedInputIndex  = index;
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

	ImGui::InputText( "Output file name", state.outputFileNameBuffer.data(), state.outputFileNameBuffer.size() );
}

void drawLayoutSection( LayoutEngine::Config& layoutConfig ) {
	using GL = GuiLimits;

	ImGui::SliderFloat( "Margin X", &layoutConfig.margin_x, GL::MarginMin, GL::MarginMax );
	ImGui::SliderFloat( "Margin Y", &layoutConfig.margin_y, GL::MarginMin, GL::MarginMax );
	ImGui::SliderFloat( "Layer spacing", &layoutConfig.layer_spacing, GL::LayerSpacingMin, GL::LayerSpacingMax );
	ImGui::SliderFloat( "Node spacing", &layoutConfig.node_spacing, GL::NodeSpacingMin, GL::NodeSpacingMax );
	ImGui::SliderFloat( "Component spacing", &layoutConfig.component_spacing, GL::ComponentSpacingMin,
	                    GL::ComponentSpacingMax );
}

void drawGraphActionSection( BlueprintViewerPanel::Result& result ) {
	if ( ImGui::Button( "Load JSON" ) ) {
		result.loadGraphRequested = true;
	}

	ImGui::SameLine();
	if ( ImGui::Button( "Apply Layout" ) ) {
		result.applyLayoutRequested = true;
	}

	ImGui::SameLine();
	if ( ImGui::Button( "Save JSON" ) ) {
		result.saveGraphRequested = true;
	}
}

void drawViewActionSection( BlueprintViewerPanel::Result& result ) {
	if ( ImGui::Button( "Reset View" ) ) {
		result.resetViewRequested = true;
	}

	ImGui::SameLine();
	if ( ImGui::Button( "Fit Graph" ) ) {
		result.fitGraphRequested = true;
	}
}

void drawGraphStatsSection( const BlueprintViewerPanel::State& state ) {
	ImGui::Text( "Nodes: %zu", state.nodeCount );
	ImGui::Text( "Edges: %zu", state.edgeCount );
	ImGui::TextWrapped( "Status: %s", state.statusMessage.c_str() );
}

void drawPathsSection( const BlueprintViewerPanel::State& state ) {
	ImGui::TextDisabled( "Input:  %s", makeCompactPathLabel( state.inputDirectory ).c_str() );
	ImGui::TextDisabled( "Output: %s", makeCompactPathLabel( state.outputDirectory ).c_str() );
}

void drawControlsHelpSection() {
	ImGui::Text( "Controls:" );
	ImGui::BulletText( "Zoom:  Wheel or Keypad +/-" );
	ImGui::BulletText( "Pan:   MMB or Ctrl + LPM" );
	ImGui::BulletText( "Reset: 'Reset View' or 'Fit Graph'" );
}

void drawHoveredNodeTooltip( const BlueprintViewerPanel::State& state ) {
	if ( ImGui::GetIO().WantCaptureMouse || !state.hoveredNode.has_value() ) {
		return;
	}

	ImGui::BeginTooltip();
	ImGui::Text( "%s", state.hoveredNode->name.c_str() );
	ImGui::TextDisabled( "Node ID: %d", state.hoveredNode->id );
	ImGui::EndTooltip();
}

}  // namespace

BlueprintViewerPanel::Result BlueprintViewerPanel::draw( State state ) {
	using GL = GuiLimits;

	Result result;

	ImGui::SetNextWindowSize( { GL::DefaultWidth, GL::DefaultHeight }, ImGuiCond_FirstUseEver );
	ImGui::Begin( "Controls" );

	drawInputSection( state, result );
	ImGui::Separator();

	drawLayoutSection( state.layoutConfig );
	ImGui::Separator();

	drawGraphActionSection( result );
	drawViewActionSection( result );
	ImGui::Separator();

	drawGraphStatsSection( state );
	ImGui::Separator();

	drawPathsSection( state );
	ImGui::Separator();

	drawControlsHelpSection();
	drawHoveredNodeTooltip( state );

	ImGui::End();
	return result;
}

}  // namespace task2
