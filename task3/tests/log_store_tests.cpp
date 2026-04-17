#include <gtest/gtest.h>

#include "log_store.hpp"
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
		        ( "task3_log_store_tests_" + std::to_string( counter.fetch_add( 1 ) ) + ".log" );

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

TEST( LogStoreTests, LoadFromFileLoadsSampleEntries ) {
	LogStore store;
	store.loadFromFile( paths::dataDir / "sample_logs.txt" );

	const auto& entries = store.entries();
	ASSERT_EQ( entries.size(), 5U );
	EXPECT_EQ( entries.front().source, "AuthService" );
	EXPECT_EQ( entries.back().source, "Payment" );
}

TEST( LogStoreTests, LoadFromFileSortsEntriesByTimestamp ) {
	ScopedTempFile file( "[2023-10-25T10:20:00] [ERROR] [Payment] Third\n"
	                     "[2023-10-25T10:00:00] [INFO] [AuthService] First\n"
	                     "[2023-10-25T10:05:12] [ERROR] [Database] Second\n" );

	LogStore store;
	store.loadFromFile( file.path() );

	const auto& entries = store.entries();
	ASSERT_EQ( entries.size(), 3U );
	EXPECT_EQ( entries[ 0 ].timestamp, ts( 2023, 10, 25, 10, 0, 0 ) );
	EXPECT_EQ( entries[ 1 ].timestamp, ts( 2023, 10, 25, 10, 5, 12 ) );
	EXPECT_EQ( entries[ 2 ].timestamp, ts( 2023, 10, 25, 10, 20, 0 ) );
	EXPECT_EQ( entries[ 0 ].message, "First" );
	EXPECT_EQ( entries[ 1 ].message, "Second" );
	EXPECT_EQ( entries[ 2 ].message, "Third" );
}

TEST( LogStoreTests, ViewReturnsEntriesAsSpan ) {
	LogStore store;
	store.loadFromFile( paths::dataDir / "sample_logs.txt" );

	const std::span< const LogEntry > view = store.view();

	ASSERT_EQ( view.size(), 5U );
	EXPECT_EQ( view.front().source, "AuthService" );
	EXPECT_EQ( view.back().source, "Payment" );
}

TEST( LogStoreTests, LoadFromFilePropagatesOpenFailure ) {
	LogStore store;
	const auto missingPath = paths::dataDir / "missing_store_input.log";

	EXPECT_THROW( static_cast< void >( store.loadFromFile( missingPath ) ), std::runtime_error );
}

TEST( LogStoreTests, LoadFromFilePreservesInputOrderForEqualTimestamps ) {
	ScopedTempFile file( "[2023-10-25T10:00:00] [INFO] [AuthService] First\n"
	                     "[2023-10-25T10:00:00] [ERROR] [Database] Second\n"
	                     "[2023-10-25T10:00:00] [WARN] [Payment] Third\n" );

	LogStore store;
	store.loadFromFile( file.path() );

	const auto& entries = store.entries();
	ASSERT_EQ( entries.size(), 3U );
	EXPECT_EQ( entries[ 0 ].message, "First" );
	EXPECT_EQ( entries[ 1 ].message, "Second" );
	EXPECT_EQ( entries[ 2 ].message, "Third" );
}

}  // namespace task3
