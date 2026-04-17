#include "blueprint_viewer_panel.hpp"

#include <imgui.h>

namespace task2 {

namespace {

[[nodiscard]] bool isValidSelection( const BlueprintViewerPanel::State& state ) {
	return state.selectedInputIndex >= 0 && state.selectedInputIndex < static_cast< int >( state.inputFiles.size() );
}

}  // namespace

BlueprintViewerPanel::Result BlueprintViewerPanel::draw( State state ) const {
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

std::string BlueprintViewerPanel::selectedInputLabel( const State& state ) {
	if ( !hasSelectedInputFile( state ) ) {
		return "<none>";
	}

	return state.inputFiles[ state.selectedInputIndex ].filename().string();
}

std::string BlueprintViewerPanel::makeCompactPathLabel( const std::filesystem::path& path ) {
	return path.parent_path().filename().string() + "/" + path.filename().string();
}

bool BlueprintViewerPanel::hasSelectedInputFile( const State& state ) {
	return isValidSelection( state );
}

void BlueprintViewerPanel::drawInputSection( State state, Result& result ) const {
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

void BlueprintViewerPanel::drawLayoutSection( LayoutEngine::Config& layoutConfig ) const {
	using GL = GuiLimits;

	ImGui::SliderFloat( "Margin X", &layoutConfig.margin_x, GL::MarginMin, GL::MarginMax );
	ImGui::SliderFloat( "Margin Y", &layoutConfig.margin_y, GL::MarginMin, GL::MarginMax );
	ImGui::SliderFloat( "Layer spacing", &layoutConfig.layer_spacing, GL::LayerSpacingMin, GL::LayerSpacingMax );
	ImGui::SliderFloat( "Node spacing", &layoutConfig.node_spacing, GL::NodeSpacingMin, GL::NodeSpacingMax );
	ImGui::SliderFloat( "Component spacing", &layoutConfig.component_spacing, GL::ComponentSpacingMin,
	                    GL::ComponentSpacingMax );
}

void BlueprintViewerPanel::drawGraphActionSection( Result& result ) const {
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

void BlueprintViewerPanel::drawViewActionSection( Result& result ) const {
	if ( ImGui::Button( "Reset View" ) ) {
		result.resetViewRequested = true;
	}

	ImGui::SameLine();
	if ( ImGui::Button( "Fit Graph" ) ) {
		result.fitGraphRequested = true;
	}
}

void BlueprintViewerPanel::drawGraphStatsSection( const State& state ) const {
	ImGui::Text( "Nodes: %zu", state.nodeCount );
	ImGui::Text( "Edges: %zu", state.edgeCount );
	ImGui::TextWrapped( "Status: %s", state.statusMessage.c_str() );
}

void BlueprintViewerPanel::drawPathsSection( const State& state ) const {
	ImGui::TextDisabled( "Input:  %s", makeCompactPathLabel( state.inputDirectory ).c_str() );
	ImGui::TextDisabled( "Output: %s", makeCompactPathLabel( state.outputDirectory ).c_str() );
}

void BlueprintViewerPanel::drawControlsHelpSection() const {
	ImGui::Text( "Controls:" );
	ImGui::BulletText( "Zoom:  Wheel or Keypad +/-" );
	ImGui::BulletText( "Pan:   MMB or Ctrl + LPM" );
	ImGui::BulletText( "Reset: 'Reset View' or 'Fit Graph'" );
}

void BlueprintViewerPanel::drawHoveredNodeTooltip( const State& state ) const {
	if ( ImGui::GetIO().WantCaptureMouse || !state.hoveredNode.has_value() ) {
		return;
	}

	ImGui::BeginTooltip();
	ImGui::Text( "%s", state.hoveredNode->name.c_str() );
	ImGui::TextDisabled( "Node ID: %d", state.hoveredNode->id );
	ImGui::EndTooltip();
}

}  // namespace task2
