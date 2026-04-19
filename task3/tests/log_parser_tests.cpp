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

using std::chrono::day;
using std::chrono::hours;
using std::chrono::minutes;
using std::chrono::month;
using std::chrono::seconds;
using std::chrono::sys_days;
using std::chrono::sys_seconds;
using std::chrono::year;

sys_seconds ts( int y, unsigned m, unsigned d, unsigned h, unsigned min, unsigned s ) {
	return sys_days{ year{ y } / month{ m } / day{ d } } + hours{ h } + minutes{ min } + seconds{ s };
}

class ScopedTempFile {
public:
	explicit ScopedTempFile( const std::string& contents ) {
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

TEST( LogParserTests, ReturnsNulloptForWhitespaceOnlyLine ) {
	auto parsed = LogParser::parseLine( "  \t \r" );

	EXPECT_FALSE( parsed.has_value() );
}

TEST( LogParserTests, TrimsTrailingCarriageReturn ) {
	const std::string line = "[2023-10-25T10:05:12] [ERROR] [Database] Connection timeout\r";

	auto parsed = LogParser::parseLine( line );

	ASSERT_TRUE( parsed.has_value() );
	EXPECT_EQ( parsed->raw_line, "[2023-10-25T10:05:12] [ERROR] [Database] Connection timeout" );
}

TEST( LogParserTests, ThrowsForInvalidFormat ) {
	const std::string line = "invalid log line";

	EXPECT_THROW( static_cast< void >( LogParser::parseLine( line ) ), std::runtime_error );
}

TEST( LogParserTests, ThrowsWhenMessageIsMissing ) {
	const std::string line = "[2023-10-25T10:05:12] [ERROR] [Database]";

	EXPECT_THROW( static_cast< void >( LogParser::parseLine( line ) ), std::runtime_error );
}

TEST( LogParserTests, ThrowsWhenBracketedSectionIsMissing ) {
	const std::string line = "[2023-10-25T10:05:12] [ERROR] Database Connection timeout";

	EXPECT_THROW( static_cast< void >( LogParser::parseLine( line ) ), std::runtime_error );
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

TEST( LogParserTests, ParseFileSkipsBlankAndWhitespaceOnlyLines ) {
	ScopedTempFile file( "\n"
	                     "   \t\n"
	                     "[2023-10-25T10:00:00] [INFO] [AuthService] Ok\n"
	                     "\r\n"
	                     "[2023-10-25T10:05:12] [ERROR] [Database] Failed\n" );

	const auto entries = LogParser::parseFile( file.path() );

	ASSERT_EQ( entries.size(), 2U );
	EXPECT_EQ( entries[ 0 ].message, "Ok" );
	EXPECT_EQ( entries[ 1 ].message, "Failed" );
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
