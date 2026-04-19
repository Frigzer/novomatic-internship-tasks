#pragma once

#include "layout_engine.hpp"

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace task2::BlueprintViewerPanel {

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

[[nodiscard]] Result draw( State state );

}  // namespace task2::BlueprintViewerPanel
