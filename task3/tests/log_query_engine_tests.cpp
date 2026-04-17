#include <gtest/gtest.h>

#include "log_query_engine.hpp"

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

TEST( LogQueryEngineTests, FiltersByLevelAndTimeRange ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.level      = LogLevel::Error;
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

}  // namespace task3