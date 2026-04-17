#pragma once

#include "layout_engine.hpp"

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace task2 {

class BlueprintViewerPanel {
public:
	static constexpr std::size_t OutputFileNameBufferSize = 256;

	struct HoveredNodeInfo {
		int id{};
		std::string name;
	};

	struct State {
		const std::vector< std::filesystem::path >& inputFiles;
		int selectedInputIndex{};
		std::array< char, OutputFileNameBufferSize >& outputFileNameBuffer;
		LayoutEngine::Config& layoutConfig;
		const std::string& statusMessage;
		std::size_t nodeCount{};
		std::size_t edgeCount{};
		const std::filesystem::path& inputDirectory;
		const std::filesystem::path& outputDirectory;
		std::optional< HoveredNodeInfo > hoveredNode;
	};

	struct Result {
		std::optional< int > selectedInputIndex;
		bool refreshInputFilesRequested{ false };
		bool loadGraphRequested{ false };
		bool applyLayoutRequested{ false };
		bool saveGraphRequested{ false };
		bool resetViewRequested{ false };
		bool fitGraphRequested{ false };
	};

	[[nodiscard]] Result draw( State state ) const;

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

	[[nodiscard]] static std::string selectedInputLabel( const State& state );
	[[nodiscard]] static std::string makeCompactPathLabel( const std::filesystem::path& path );
	[[nodiscard]] static bool hasSelectedInputFile( const State& state );

	void drawInputSection( State state, Result& result ) const;
	void drawLayoutSection( LayoutEngine::Config& layoutConfig ) const;
	void drawGraphActionSection( Result& result ) const;
	void drawViewActionSection( Result& result ) const;
	void drawGraphStatsSection( const State& state ) const;
	void drawPathsSection( const State& state ) const;
	void drawControlsHelpSection() const;
	void drawHoveredNodeTooltip( const State& state ) const;
};

}  // namespace task2
