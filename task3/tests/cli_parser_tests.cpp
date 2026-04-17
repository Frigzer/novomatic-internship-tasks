#include <gtest/gtest.h>

#include "cli_parser.hpp"

#include <string>
#include <vector>

namespace task3 {
namespace {

TEST( CliParserTests, ReturnsInteractiveModeWhenNoArgumentsAreProvided ) {
	CliParser parser;
	const std::vector< std::string > args{ "task3" };

	const auto result = parser.parse( args );

	EXPECT_EQ( result.mode, CliMode::Interactive );
}

TEST( CliParserTests, ReturnsHelpModeWhenHelpOptionIsProvided ) {
	CliParser parser;
	const std::vector< std::string > args{ "task3", "--help" };

	const auto result = parser.parse( args );

	EXPECT_EQ( result.mode, CliMode::ShowHelp );
}

TEST( CliParserTests, ParsesLogFileAndFilters ) {
	CliParser parser;
	const std::vector< std::string > args{ "task3",    "logs.txt",  "--level", "ERROR",  "--source",
	                                       "Database", "--message", "timeout", "--count" };

	const auto result = parser.parse( args );

	ASSERT_EQ( result.mode, CliMode::Execute );
	EXPECT_EQ( result.options.logFile, "logs.txt" );
	ASSERT_TRUE( result.options.query.level.has_value() );
	EXPECT_EQ( *result.options.query.level, LogLevel::Error );
	ASSERT_TRUE( result.options.query.source.has_value() );
	EXPECT_EQ( *result.options.query.source, "Database" );
	ASSERT_TRUE( result.options.query.messageContains.has_value() );
	EXPECT_EQ( *result.options.query.messageContains, "timeout" );
	EXPECT_TRUE( result.options.countOnly );
}

TEST( CliParserTests, ParsesExactTimestampIntoSinglePointTimeRange ) {
	CliParser parser;
	const std::vector< std::string > args{ "task3", "logs.txt", "--timestamp", "2023-10-25T10:05:12" };

	const auto result = parser.parse( args );

	ASSERT_EQ( result.mode, CliMode::Execute );
	ASSERT_TRUE( result.options.query.timeRange.has_value() );
	EXPECT_EQ( result.options.query.timeRange->from, result.options.query.timeRange->to );
}

TEST( CliParserTests, ReturnsErrorForUnknownOption ) {
	CliParser parser;
	const std::vector< std::string > args{ "task3", "logs.txt", "--unknown" };

	const auto result = parser.parse( args );

	EXPECT_EQ( result.mode, CliMode::Error );
	EXPECT_NE( result.message.find( "Unknown option" ), std::string::npos );
}

TEST( CliParserTests, ReturnsErrorForTimestampAndRangeCombination ) {
	CliParser parser;
	const std::vector< std::string > args{
	    "task3", "logs.txt",           "--timestamp", "2023-10-25T10:05:12", "--from", "2023-10-25T10:00:00",
	    "--to",  "2023-10-25T10:10:00" };

	const auto result = parser.parse( args );

	EXPECT_EQ( result.mode, CliMode::Error );
	EXPECT_NE( result.message.find( "Use either --timestamp or --from/--to" ), std::string::npos );
}

TEST( CliParserTests, UsageContainsSourceOptionOnlyOnce ) {
	CliParser parser;
	const std::string usage = parser.usage( "task3" );

	const std::string needle = "--source <source>";
	const auto first         = usage.find( needle );
	ASSERT_NE( first, std::string::npos );
	EXPECT_EQ( usage.find( needle, first + 1 ), std::string::npos );
}

}  // namespace
}  // namespace task3
