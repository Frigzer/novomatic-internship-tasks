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

TEST( LogFileResolverTests, ResolvesPlainFilenameFromProjectDataDirectory ) {
    LogFileResolver resolver;

    const std::filesystem::path resolved = resolver.resolve( "sample_logs.txt" );

    EXPECT_EQ( resolved, std::filesystem::weakly_canonical( paths::dataDir / "sample_logs.txt" ) );
}

TEST( LogFileResolverTests, ResolvesExistingRelativePathFromCurrentWorkingDirectory ) {
    CurrentPathGuard guard;
    const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "task3_resolver_relative";
    std::filesystem::create_directories( tempDir / "nested" );
    std::ofstream( tempDir / "nested" / "logs.txt" ) << "test\n";
    std::filesystem::current_path( tempDir );

    LogFileResolver resolver;
    const std::filesystem::path resolved = resolver.resolve( std::filesystem::path( "nested" ) / "logs.txt" );

    EXPECT_EQ( resolved, std::filesystem::weakly_canonical( tempDir / "nested" / "logs.txt" ) );
    std::filesystem::remove_all( tempDir );
}

TEST( LogFileResolverTests, ResolvesFilenameFromCurrentWorkingDirectoryDataSubdirectory ) {
    CurrentPathGuard guard;
    const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "task3_resolver_data_subdir";
    std::filesystem::create_directories( tempDir / "data" );
    std::ofstream( tempDir / "data" / "runtime_logs.txt" ) << "test\n";
    std::filesystem::current_path( tempDir );

    LogFileResolver resolver;
    const std::filesystem::path resolved = resolver.resolve( "runtime_logs.txt" );

    EXPECT_EQ( resolved, std::filesystem::weakly_canonical( tempDir / "data" / "runtime_logs.txt" ) );
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
