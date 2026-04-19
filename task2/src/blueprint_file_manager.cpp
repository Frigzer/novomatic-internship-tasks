#include "blueprint_file_manager.hpp"

#include "json_io.hpp"

#include <algorithm>
#include <stdexcept>

namespace task2 {

BlueprintFileManager::BlueprintFileManager( std::filesystem::path inputDirectory, std::filesystem::path outputDirectory,
                                            std::filesystem::path defaultInputPath,
                                            std::filesystem::path defaultOutputPath )
    : inputDirectory_( std::move( inputDirectory ) ),
      outputDirectory_( std::move( outputDirectory ) ),
      inputPath_( std::move( defaultInputPath ) ),
      outputPath_( std::move( defaultOutputPath ) ) {}

void BlueprintFileManager::ensureDirectoriesExist() const {
	std::filesystem::create_directories( inputDirectory_ );
	std::filesystem::create_directories( outputDirectory_ );
}

void BlueprintFileManager::refreshInputFiles() {
	inputFiles_.clear();

	if ( !std::filesystem::exists( inputDirectory_ ) ) {
		selectedInputIndex_ = -1;
		return;
	}

	for ( const auto& entry : std::filesystem::directory_iterator( inputDirectory_ ) ) {
		if ( entry.is_regular_file() && isJsonFile( entry.path() ) ) {
			inputFiles_.push_back( entry.path() );
		}
	}

	std::ranges::sort( inputFiles_, []( const auto& left, const auto& right ) {
		return left.filename().string() < right.filename().string();
	} );

	syncSelectedInputFile();
}

bool BlueprintFileManager::hasSelectedInputFile() const {
	return selectedInputIndex_ >= 0 && selectedInputIndex_ < static_cast< int >( inputFiles_.size() );
}

const std::vector< std::filesystem::path >& BlueprintFileManager::inputFiles() const noexcept {
	return inputFiles_;
}

int BlueprintFileManager::selectedInputIndex() const noexcept {
	return selectedInputIndex_;
}

void BlueprintFileManager::setSelectedInputIndex( int index ) {
	if ( index < 0 || index >= static_cast< int >( inputFiles_.size() ) ) {
		throw std::out_of_range( "Invalid input file index" );
	}

	selectedInputIndex_ = index;
	inputPath_          = inputFiles_[ selectedInputIndex_ ];
}

std::string BlueprintFileManager::selectedInputLabel() const {
	if ( !hasSelectedInputFile() ) {
		return "<none>";
	}

	return inputFiles_[ selectedInputIndex_ ].filename().string();
}

const std::filesystem::path& BlueprintFileManager::selectedInputPath() const {
	if ( !hasSelectedInputFile() ) {
		throw std::runtime_error( "No input file selected" );
	}

	return inputPath_;
}

const std::filesystem::path& BlueprintFileManager::outputPath() const noexcept {
	return outputPath_;
}

std::string BlueprintFileManager::outputFileName() const {
	return outputPath_.filename().string();
}

void BlueprintFileManager::setOutputFileName( std::string_view fileName ) {
	if ( fileName.empty() ) {
		throw std::runtime_error( "Output file name is empty" );
	}

	outputPath_ = outputDirectory_ / std::string( fileName );
}

Graph BlueprintFileManager::loadSelectedGraph() const {
	return JsonGraphIO::loadFromFile( selectedInputPath() );
}

void BlueprintFileManager::saveGraph( const Graph& graph ) const {
	JsonGraphIO::saveToFile( graph, outputPath_ );
}

void BlueprintFileManager::syncSelectedInputFile() {
	if ( inputFiles_.empty() ) {
		selectedInputIndex_ = -1;
		inputPath_.clear();
		return;
	}

	if ( const auto it = std::ranges::find( inputFiles_, inputPath_ ); it != inputFiles_.end() ) {
		selectedInputIndex_ = static_cast< int >( std::distance( inputFiles_.begin(), it ) );
	} else {
		selectedInputIndex_ = 0;
		inputPath_          = inputFiles_.front();
	}
}

bool BlueprintFileManager::isJsonFile( const std::filesystem::path& path ) {
	return path.extension() == ".json";
}

}  // namespace task2
