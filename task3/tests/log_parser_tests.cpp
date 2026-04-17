#include <gtest/gtest.h>

#include "log_parser.hpp"
#include "paths.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace task3 {
namespace {

using namespace std::chrono;

std::chrono::sys_seconds ts( int year, unsigned month, unsigned day, int hour, int minute, int second ) {
	return sys_days{ std::chrono::year{ year } / month / day } + hours{ hour } + minutes{ minute } + seconds{ second };
}

class ScopedTempFile {
public:
	explicit ScopedTempFile( std::string contents ) {
		static std::atomic< int > counter{ 0 };
		path_ = std::filesystem::temp_directory_path() /
		        ( "task3_log_parser_tests_" + std::to_string( counter.fetch_add( 1 ) ) + ".log" );

		std::ofstream out( path_ );
		out << contents;
	}

	~ScopedTempFile() {
		std::error_code ec;
		std::filesystem::remove( path_, ec );
	}

	const std::filesystem::path& path() const { return path_; }

private:
	std::filesystem::path path_;
};

}  // namespace

TEST( LogParserTests, ParsesValidLine ) {
	const std::string line = "[2023-10-25T10:05:12] [ERROR] [Database] Connection timeout";

	auto parsed = LogParser::parseLine( line );

	ASSERT_TRUE( parsed.has_value() );
	EXPECT_EQ( parsed->timestamp, ts( 2023, 10, 25, 10, 5, 12 ) );
	EXPECT_EQ( parsed->level, LogLevel::Error );
	EXPECT_EQ( parsed->source, "Database" );
	EXPECT_EQ( parsed->message, "Connection timeout" );
	EXPECT_EQ( parsed->raw_line, line );
}

TEST( LogParserTests, ReturnsNulloptForEmptyLine ) {
	const std::string line;

	auto parsed = LogParser::parseLine( line );

	EXPECT_FALSE( parsed.has_value() );
}

TEST( LogParserTests, ReturnsNulloptForInvalidFormat ) {
	const std::string line = "invalid log line";

	auto parsed = LogParser::parseLine( line );

	EXPECT_FALSE( parsed.has_value() );
}

TEST( LogParserTests, ReturnsNulloptWhenMessageIsMissing ) {
	const std::string line = "[2023-10-25T10:05:12] [ERROR] [Database]";

	auto parsed = LogParser::parseLine( line );

	EXPECT_FALSE( parsed.has_value() );
}

TEST( LogParserTests, ReturnsNulloptWhenBracketedSectionIsMissing ) {
	const std::string line = "[2023-10-25T10:05:12] [ERROR] Database Connection timeout";

	auto parsed = LogParser::parseLine( line );

	EXPECT_FALSE( parsed.has_value() );
}

TEST( LogParserTests, ThrowsForInvalidCalendarDate ) {
	const std::string line = "[2023-02-30T10:05:12] [ERROR] [Database] Connection timeout";

	EXPECT_THROW( static_cast< void >( LogParser::parseLine( line ) ), std::runtime_error );
}

TEST( LogParserTests, ThrowsForInvalidTimeValue ) {
	const std::string line = "[2023-10-25T25:05:12] [ERROR] [Database] Connection timeout";

	EXPECT_THROW( static_cast< void >( LogParser::parseLine( line ) ), std::runtime_error );
}

TEST( LogParserTests, ThrowsForUnknownLogLevel ) {
	const std::string line = "[2023-10-25T10:05:12] [NOTICE] [Database] Connection timeout";

	EXPECT_THROW( static_cast< void >( LogParser::parseLine( line ) ), std::runtime_error );
}

TEST( LogParserTests, ParseFileReadsSampleLogs ) {
	const auto entries = LogParser::parseFile( paths::dataDir / "sample_logs.txt" );

	ASSERT_EQ( entries.size(), 5U );
	EXPECT_EQ( entries.front().source, "AuthService" );
	EXPECT_EQ( entries.front().level, LogLevel::Info );
	EXPECT_EQ( entries.back().source, "Payment" );
	EXPECT_EQ( entries.back().message, "Transaction 12345 processed" );
}

TEST( LogParserTests, ParseFileThrowsForMissingFile ) {
	const auto missingPath = paths::dataDir / "does_not_exist.log";

	try {
		static_cast< void >( LogParser::parseFile( missingPath ) );
		FAIL() << "Expected std::runtime_error";
	} catch ( const std::runtime_error& ex ) {
		EXPECT_NE( std::string( ex.what() ).find( "Failed to open log file" ), std::string::npos );
	}
}

TEST( LogParserTests, ParseFileIncludesLineNumberForMalformedStructure ) {
	ScopedTempFile file( "[2023-10-25T10:00:00] [INFO] [AuthService] Ok\n"
	                     "broken line\n" );

	try {
		static_cast< void >( LogParser::parseFile( file.path() ) );
		FAIL() << "Expected std::runtime_error";
	} catch ( const std::runtime_error& ex ) {
		const std::string message = ex.what();
		EXPECT_NE( message.find( "line 2" ), std::string::npos );
		EXPECT_NE( message.find( "Invalid log line format" ), std::string::npos );
	}
}

TEST( LogParserTests, ParseFileIncludesLineNumberForInvalidTimestamp ) {
	ScopedTempFile file( "[2023-10-25T10:00:00] [INFO] [AuthService] Ok\n"
	                     "[2023-99-25T10:05:12] [ERROR] [Database] Bad timestamp\n" );

	try {
		static_cast< void >( LogParser::parseFile( file.path() ) );
		FAIL() << "Expected std::runtime_error";
	} catch ( const std::runtime_error& ex ) {
		const std::string message = ex.what();
		EXPECT_NE( message.find( "line 2" ), std::string::npos );
		EXPECT_NE( message.find( "Invalid calendar date" ), std::string::npos );
	}
}

}  // namespace task3
