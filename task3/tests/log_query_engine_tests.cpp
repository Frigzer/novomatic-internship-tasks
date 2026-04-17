#include <gtest/gtest.h>

#include "log_query_engine.hpp"

#include <chrono>
#include <vector>

namespace task3 {
namespace {

using namespace std::chrono;

std::chrono::sys_seconds ts( int year, unsigned month, unsigned day, int hour, int minute, int second ) {
	return sys_days{ std::chrono::year{ year } / month / day } + hours{ hour } + minutes{ minute } + seconds{ second };
}

std::vector< LogEntry > makeSampleEntries() {
	return { { ts( 2023, 10, 25, 10, 0, 0 ), LogLevel::Info, "AuthService", "User logged in successfully",
	           "[2023-10-25T10:00:00] [INFO] [AuthService] User logged in successfully" },

	         { ts( 2023, 10, 25, 10, 5, 12 ), LogLevel::Error, "Database", "Connection timeout",
	           "[2023-10-25T10:05:12] [ERROR] [Database] Connection timeout" },

	         { ts( 2023, 10, 25, 10, 15, 30 ), LogLevel::Warn, "AuthService", "Multiple failed login attempts",
	           "[2023-10-25T10:15:30] [WARN] [AuthService] Multiple failed login attempts" },

	         { ts( 2023, 10, 25, 10, 20, 0 ), LogLevel::Error, "Payment", "Transaction rejected: insufficient funds",
	           "[2023-10-25T10:20:00] [ERROR] [Payment] Transaction rejected: insufficient funds" },

	         { ts( 2023, 10, 25, 10, 25, 0 ), LogLevel::Info, "Payment", "Transaction 12345 processed",
	           "[2023-10-25T10:25:00] [INFO] [Payment] Transaction 12345 processed" } };
}

}  // namespace

TEST( LogQueryEngineTests, ReturnsAllEntriesForEmptyQuery ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	const auto results = engine.execute( LogQuery{} );

	ASSERT_EQ( results.size(), entries.size() );
	for ( std::size_t i = 0; i < entries.size(); ++i ) {
		EXPECT_EQ( results[ i ], &entries[ i ] );
	}
}

TEST( LogQueryEngineTests, FiltersBySource ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.source = "AuthService";

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 2U );
	EXPECT_EQ( results[ 0 ]->source, "AuthService" );
	EXPECT_EQ( results[ 1 ]->source, "AuthService" );
}

TEST( LogQueryEngineTests, FiltersByLevelOnly ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.level = LogLevel::Error;

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 2U );
	EXPECT_EQ( results[ 0 ]->source, "Database" );
	EXPECT_EQ( results[ 1 ]->source, "Payment" );
}

TEST( LogQueryEngineTests, FiltersByInclusiveTimeRange ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.timeRange = TimeRange{ ts( 2023, 10, 25, 10, 5, 12 ), ts( 2023, 10, 25, 10, 20, 0 ) };

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 3U );
	EXPECT_EQ( results[ 0 ]->timestamp, ts( 2023, 10, 25, 10, 5, 12 ) );
	EXPECT_EQ( results[ 1 ]->timestamp, ts( 2023, 10, 25, 10, 15, 30 ) );
	EXPECT_EQ( results[ 2 ]->timestamp, ts( 2023, 10, 25, 10, 20, 0 ) );
}

TEST( LogQueryEngineTests, FiltersByLevelAndTimeRange ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.level     = LogLevel::Error;
	query.timeRange = TimeRange{ ts( 2023, 10, 25, 10, 0, 0 ), ts( 2023, 10, 25, 10, 10, 0 ) };

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 1U );
	EXPECT_EQ( results[ 0 ]->source, "Database" );
}

TEST( LogQueryEngineTests, FiltersByPartialMessage ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.messageContains = "Transaction";

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 2U );
	EXPECT_EQ( results[ 0 ]->source, "Payment" );
	EXPECT_EQ( results[ 1 ]->source, "Payment" );
}

TEST( LogQueryEngineTests, FiltersByCombinedCriteria ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.level           = LogLevel::Error;
	query.source          = "Payment";
	query.messageContains = "insufficient";
	query.timeRange       = TimeRange{ ts( 2023, 10, 25, 10, 10, 0 ), ts( 2023, 10, 25, 10, 25, 0 ) };

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 1U );
	EXPECT_EQ( results[ 0 ]->source, "Payment" );
	EXPECT_EQ( results[ 0 ]->message, "Transaction rejected: insufficient funds" );
}

TEST( LogQueryEngineTests, ReturnsEmptyWhenNoEntriesMatch ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.source = "Billing";

	EXPECT_TRUE( engine.execute( query ).empty() );
}

TEST( LogQueryEngineTests, SupportsExactTimestampUsingSinglePointRange ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.timeRange = TimeRange{ ts( 2023, 10, 25, 10, 15, 30 ), ts( 2023, 10, 25, 10, 15, 30 ) };

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 1U );
	EXPECT_EQ( results[ 0 ]->source, "AuthService" );
	EXPECT_EQ( results[ 0 ]->message, "Multiple failed login attempts" );
}

TEST( LogQueryEngineTests, ReturnsEmptyForReversedTimeRange ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.timeRange = TimeRange{ ts( 2023, 10, 25, 10, 30, 0 ), ts( 2023, 10, 25, 10, 0, 0 ) };

	EXPECT_TRUE( engine.execute( query ).empty() );
}

TEST( LogQueryEngineTests, PartialMessageMatchIsCaseSensitive ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.messageContains = "transaction";

	EXPECT_TRUE( engine.execute( query ).empty() );
}

}  // namespace task3
