#include <gtest/gtest.h>

#include "cli_parser.hpp"
#include "log_file_resolver.hpp"
#include "paths.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace task3 {
namespace {

class CurrentPathGuard {
public:
	CurrentPathGuard() : original_( std::filesystem::current_path() ) {}

	~CurrentPathGuard() { std::filesystem::current_path( original_ ); }

private:
	std::filesystem::path original_;
};

TEST( LogFileResolverTests, FallsBackToProjectDataDirectoryWhenNotFoundInCurrentWorkingDirectory ) {
	const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "task3_resolver_project_fallback";
	std::filesystem::remove_all( tempDir );
	std::filesystem::create_directories( tempDir );

	{
		CurrentPathGuard guard;
		std::filesystem::current_path( tempDir );

		LogFileResolver resolver;
		const std::filesystem::path resolved = resolver.resolve( "sample_logs.txt" );

		EXPECT_EQ( resolved, std::filesystem::weakly_canonical( paths::dataDir / "sample_logs.txt" ) );
	}

	std::filesystem::remove_all( tempDir );
}

TEST( LogFileResolverTests, ResolvesExistingRelativePathFromCurrentWorkingDirectory ) {
	const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "task3_resolver_relative";
	std::filesystem::remove_all( tempDir );
	std::filesystem::create_directories( tempDir / "nested" );
	std::ofstream( tempDir / "nested" / "logs.txt" ) << "test\n";

	{
		CurrentPathGuard guard;
		std::filesystem::current_path( tempDir );

		LogFileResolver resolver;
		const std::filesystem::path resolved = resolver.resolve( std::filesystem::path( "nested" ) / "logs.txt" );

		EXPECT_EQ( resolved, std::filesystem::weakly_canonical( tempDir / "nested" / "logs.txt" ) );
	}

	std::filesystem::remove_all( tempDir );
}

TEST( LogFileResolverTests, ResolvesFilenameFromCurrentWorkingDirectoryDataSubdirectory ) {
	const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "task3_resolver_data_subdir";
	std::filesystem::remove_all( tempDir );
	std::filesystem::create_directories( tempDir / "data" );
	std::ofstream( tempDir / "data" / "runtime_logs.txt" ) << "test\n";

	{
		CurrentPathGuard guard;
		std::filesystem::current_path( tempDir );

		LogFileResolver resolver;
		const std::filesystem::path resolved = resolver.resolve( "runtime_logs.txt" );

		EXPECT_EQ( resolved, std::filesystem::weakly_canonical( tempDir / "data" / "runtime_logs.txt" ) );
	}

	std::filesystem::remove_all( tempDir );
}

TEST( LogFileResolverTests, PrefersCurrentWorkingDirectoryDataOverProjectData ) {
	const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "task3_resolver_runtime_data";
	std::filesystem::remove_all( tempDir );
	std::filesystem::create_directories( tempDir / "data" );
	std::ofstream( tempDir / "data" / "sample_logs.txt" ) << "runtime\n";

	{
		CurrentPathGuard guard;
		std::filesystem::current_path( tempDir );

		LogFileResolver resolver;
		const std::filesystem::path resolved = resolver.resolve( "sample_logs.txt" );

		EXPECT_EQ( resolved, std::filesystem::weakly_canonical( tempDir / "data" / "sample_logs.txt" ) );
	}

	std::filesystem::remove_all( tempDir );
}

TEST( CliParserTests, UsageMentionsConvenientDataLookup ) {
	CliParser parser;
	const std::string usage = parser.usage( "task3" );

	EXPECT_NE( usage.find( "just a file name" ), std::string::npos );
	EXPECT_NE( usage.find( "data/<file>" ), std::string::npos );
}

}  // namespace
}  // namespace task3
