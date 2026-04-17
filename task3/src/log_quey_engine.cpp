#include "log_query_engine.hpp"

#include <algorithm>
#include <ranges>

namespace task3 {
namespace {

struct TimestampLess {
	bool operator()( const LogEntry& entry, const std::chrono::sys_seconds& timestamp ) const {
		return entry.timestamp < timestamp;
	}

	bool operator()( const std::chrono::sys_seconds& timestamp, const LogEntry& entry ) const {
		return timestamp < entry.timestamp;
	}
};

}  // namespace

LogQueryEngine::LogQueryEngine( std::span< const LogEntry > entries ) : entries_( entries ) {}

std::vector< const LogEntry* > LogQueryEngine::execute( const LogQuery& query ) const {
	auto begin = entries_.begin();
	auto end   = entries_.end();

	if ( query.timeRange.has_value() ) {
		const auto from = query.timeRange->from;
		const auto to   = query.timeRange->to;

		begin = std::lower_bound( begin, end, from, TimestampLess{} );
		end   = std::upper_bound( begin, end, to, TimestampLess{} );
	}

	std::vector< const LogEntry* > results;
	for ( auto it = begin; it != end; ++it ) {
		if ( matches( *it, query ) ) {
			results.push_back( &( *it ) );
		}
	}

	return results;
}

bool LogQueryEngine::matches( const LogEntry& entry, const LogQuery& query ) const {
	if ( query.level.has_value() && entry.level != *query.level ) {
		return false;
	}

	if ( query.source.has_value() && entry.source != *query.source ) {
		return false;
	}

	if ( query.messageContains.has_value() ) {
		if ( entry.message.find( *query.messageContains ) == std::string::npos ) {
			return false;
		}
	}

	if ( query.timeRange.has_value() ) {
		if ( entry.timestamp < query.timeRange->from || entry.timestamp > query.timeRange->to ) {
			return false;
		}
	}

	return true;
}

}  // namespace task3