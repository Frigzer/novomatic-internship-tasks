#pragma once

#include "graph.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace task2 {

class BlueprintFileManager {
public:
	BlueprintFileManager( std::filesystem::path inputDirectory, std::filesystem::path outputDirectory,
	                      std::filesystem::path defaultInputPath, std::filesystem::path defaultOutputPath );

	void ensureDirectoriesExist() const;
	void refreshInputFiles();

	[[nodiscard]] bool hasSelectedInputFile() const;
	[[nodiscard]] const std::vector< std::filesystem::path >& inputFiles() const noexcept;
	[[nodiscard]] int selectedInputIndex() const noexcept;
	void setSelectedInputIndex( int index );
	[[nodiscard]] std::string selectedInputLabel() const;
	[[nodiscard]] const std::filesystem::path& selectedInputPath() const;

	[[nodiscard]] const std::filesystem::path& outputPath() const noexcept;
	[[nodiscard]] std::string outputFileName() const;
	void setOutputFileName( std::string_view fileName );

	[[nodiscard]] Graph loadSelectedGraph() const;
	void saveGraph( const Graph& graph ) const;

private:
	void syncSelectedInputFile();
	[[nodiscard]] static bool isJsonFile( const std::filesystem::path& path );

	std::filesystem::path inputDirectory_;
	std::filesystem::path outputDirectory_;
	std::filesystem::path inputPath_;
	std::filesystem::path outputPath_;
	std::vector< std::filesystem::path > inputFiles_;
	int selectedInputIndex_{ -1 };
};

}  // namespace task2
