#include <gtest/gtest.h>

#include "log_query_engine.hpp"

#include <chrono>
#include <vector>

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

std::vector< LogEntry > makeSampleEntries() {
	return { { .timestamp = ts( 2023, 10, 25, 10, 0, 0 ),
	           .level     = LogLevel::Info,
	           .source    = "AuthService",
	           .message   = "User logged in successfully",
	           .raw_line  = "[2023-10-25T10:00:00] [INFO] [AuthService] User logged in successfully" },

	         { .timestamp = ts( 2023, 10, 25, 10, 5, 12 ),
	           .level     = LogLevel::Error,
	           .source    = "Database",
	           .message   = "Connection timeout",
	           .raw_line  = "[2023-10-25T10:05:12] [ERROR] [Database] Connection timeout" },

	         { .timestamp = ts( 2023, 10, 25, 10, 15, 30 ),
	           .level     = LogLevel::Warn,
	           .source    = "AuthService",
	           .message   = "Multiple failed login attempts",
	           .raw_line  = "[2023-10-25T10:15:30] [WARN] [AuthService] Multiple failed login attempts" },

	         { .timestamp = ts( 2023, 10, 25, 10, 20, 0 ),
	           .level     = LogLevel::Error,
	           .source    = "Payment",
	           .message   = "Transaction rejected: insufficient funds",
	           .raw_line  = "[2023-10-25T10:20:00] [ERROR] [Payment] Transaction rejected: insufficient funds" },

	         { .timestamp = ts( 2023, 10, 25, 10, 25, 0 ),
	           .level     = LogLevel::Info,
	           .source    = "Payment",
	           .message   = "Transaction 12345 processed",
	           .raw_line  = "[2023-10-25T10:25:00] [INFO] [Payment] Transaction 12345 processed" } };
}

std::vector< LogEntry > makeUnsortedEntries() {
	return { { .timestamp = ts( 2023, 10, 25, 10, 20, 0 ),
	           .level     = LogLevel::Error,
	           .source    = "Payment",
	           .message   = "Third",
	           .raw_line  = "[2023-10-25T10:20:00] [ERROR] [Payment] Third" },
	         { .timestamp = ts( 2023, 10, 25, 10, 0, 0 ),
	           .level     = LogLevel::Info,
	           .source    = "AuthService",
	           .message   = "First",
	           .raw_line  = "[2023-10-25T10:00:00] [INFO] [AuthService] First" },
	         { .timestamp = ts( 2023, 10, 25, 10, 5, 12 ),
	           .level     = LogLevel::Error,
	           .source    = "Database",
	           .message   = "Second",
	           .raw_line  = "[2023-10-25T10:05:12] [ERROR] [Database] Second" } };
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
	query.timeRange = TimeRange{ .from = ts( 2023, 10, 25, 10, 5, 12 ), .to = ts( 2023, 10, 25, 10, 20, 0 ) };

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
	query.timeRange = TimeRange{ .from = ts( 2023, 10, 25, 10, 0, 0 ), .to = ts( 2023, 10, 25, 10, 10, 0 ) };

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
	query.timeRange       = TimeRange{ .from = ts( 2023, 10, 25, 10, 10, 0 ), .to = ts( 2023, 10, 25, 10, 25, 0 ) };

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
	query.timeRange = TimeRange{ .from = ts( 2023, 10, 25, 10, 15, 30 ), .to = ts( 2023, 10, 25, 10, 15, 30 ) };

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 1U );
	EXPECT_EQ( results[ 0 ]->source, "AuthService" );
	EXPECT_EQ( results[ 0 ]->message, "Multiple failed login attempts" );
}

TEST( LogQueryEngineTests, ReturnsEmptyForReversedTimeRange ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.timeRange = TimeRange{ .from = ts( 2023, 10, 25, 10, 30, 0 ), .to = ts( 2023, 10, 25, 10, 0, 0 ) };

	EXPECT_TRUE( engine.execute( query ).empty() );
}

TEST( LogQueryEngineTests, PartialMessageMatchIsCaseInsensitive ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.messageContains = "transaction";

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 2U );
	EXPECT_EQ( results[ 0 ]->source, "Payment" );
	EXPECT_EQ( results[ 1 ]->source, "Payment" );
}

TEST( LogQueryEngineTests, CanUseCaseSensitiveMessageMatchingWhenRequested ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.messageContains      = "transaction";
	query.messageCaseSensitive = true;

	const auto results = engine.execute( query );

	EXPECT_TRUE( results.empty() );
}

TEST( LogQueryEngineTests, CaseSensitiveMessageMatchingStillFindsExactCasing ) {
	const auto entries = makeSampleEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.messageContains      = "Transaction";
	query.messageCaseSensitive = true;

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 2U );
}

TEST( LogQueryEngineTests, SupportsUnsortedInputWithoutMissingTimeRangeResults ) {
	const auto entries = makeUnsortedEntries();
	LogQueryEngine engine( entries );

	LogQuery query;
	query.timeRange = TimeRange{ .from = ts( 2023, 10, 25, 10, 0, 0 ), .to = ts( 2023, 10, 25, 10, 5, 12 ) };

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 2U );
	EXPECT_EQ( results[ 0 ]->message, "First" );
	EXPECT_EQ( results[ 1 ]->message, "Second" );
}

TEST( LogQueryEngineTests, PreservesOriginalOrderForEqualTimestamps ) {
	std::vector< LogEntry > entries{ { .timestamp = ts( 2023, 10, 25, 10, 0, 0 ),
	                                   .level     = LogLevel::Error,
	                                   .source    = "A",
	                                   .message   = "alpha",
	                                   .raw_line  = "[2023-10-25T10:00:00] [ERROR] [A] alpha" },
	                                 { .timestamp = ts( 2023, 10, 25, 10, 0, 0 ),
	                                   .level     = LogLevel::Info,
	                                   .source    = "B",
	                                   .message   = "beta",
	                                   .raw_line  = "[2023-10-25T10:00:00] [INFO] [B] beta" },
	                                 { .timestamp = ts( 2023, 10, 25, 10, 0, 0 ),
	                                   .level     = LogLevel::Warn,
	                                   .source    = "C",
	                                   .message   = "gamma",
	                                   .raw_line  = "[2023-10-25T10:00:00] [WARN] [C] gamma" } };

	LogQueryEngine engine( entries );
	const auto results = engine.execute( LogQuery{} );

	ASSERT_EQ( results.size(), 3U );
	EXPECT_EQ( results[ 0 ]->source, "A" );
	EXPECT_EQ( results[ 1 ]->source, "B" );
	EXPECT_EQ( results[ 2 ]->source, "C" );
}

TEST( LogQueryEngineTests, PreservesOriginalOrderForEqualTimestampsAfterIndexedFiltering ) {
	std::vector< LogEntry > entries{ { .timestamp = ts( 2023, 10, 25, 10, 0, 0 ),
	                                   .level     = LogLevel::Error,
	                                   .source    = "Auth",
	                                   .message   = "first timeout",
	                                   .raw_line  = "[2023-10-25T10:00:00] [ERROR] [Auth] first timeout" },
	                                 { .timestamp = ts( 2023, 10, 25, 10, 0, 0 ),
	                                   .level     = LogLevel::Error,
	                                   .source    = "Billing",
	                                   .message   = "second timeout",
	                                   .raw_line  = "[2023-10-25T10:00:00] [ERROR] [Billing] second timeout" },
	                                 { .timestamp = ts( 2023, 10, 25, 10, 0, 0 ),
	                                   .level     = LogLevel::Error,
	                                   .source    = "Cache",
	                                   .message   = "third timeout",
	                                   .raw_line  = "[2023-10-25T10:00:00] [ERROR] [Cache] third timeout" },
	                                 { .timestamp = ts( 2023, 10, 25, 10, 1, 0 ),
	                                   .level     = LogLevel::Info,
	                                   .source    = "Other",
	                                   .message   = "later",
	                                   .raw_line  = "[2023-10-25T10:01:00] [INFO] [Other] later" } };

	LogQueryEngine engine( entries );

	LogQuery query;
	query.level           = LogLevel::Error;
	query.messageContains = "TIMEOUT";

	const auto results = engine.execute( query );

	ASSERT_EQ( results.size(), 3U );
	EXPECT_EQ( results[ 0 ]->source, "Auth" );
	EXPECT_EQ( results[ 1 ]->source, "Billing" );
	EXPECT_EQ( results[ 2 ]->source, "Cache" );
}

}  // namespace task3
