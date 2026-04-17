#include "log_query_engine.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <string_view>

namespace task3 {
namespace {

using EntryPtr = const LogEntry*;

struct EntryPtrTimestampLess {
	bool operator()( EntryPtr lhs, EntryPtr rhs ) const noexcept { return lhs->timestamp < rhs->timestamp; }

	bool operator()( EntryPtr entry, const std::chrono::sys_seconds& timestamp ) const noexcept {
		return entry->timestamp < timestamp;
	}

	bool operator()( const std::chrono::sys_seconds& timestamp, EntryPtr entry ) const noexcept {
		return timestamp < entry->timestamp;
	}
};

[[nodiscard]] bool iequals( char lhs, char rhs ) noexcept {
	return std::tolower( static_cast< unsigned char >( lhs ) ) == std::tolower( static_cast< unsigned char >( rhs ) );
}

[[nodiscard]] bool containsCaseInsensitive( std::string_view text, std::string_view needle ) noexcept {
	if ( needle.empty() ) {
		return true;
	}

	const auto it = std::search( text.begin(), text.end(), needle.begin(), needle.end(), iequals );
	return it != text.end();
}

}  // namespace

LogQueryEngine::LogQueryEngine( std::span< const LogEntry > entries ) : entries_( entries ) {
	orderedByTimestamp_.reserve( entries_.size() );
	for ( const LogEntry& entry : entries_ ) {
		auto* ptr = &entry;
		orderedByTimestamp_.push_back( ptr );
		byLevel_[ entry.level ].push_back( ptr );
		bySource_[ entry.source ].push_back( ptr );
	}

	std::ranges::stable_sort( orderedByTimestamp_, EntryPtrTimestampLess{} );
}

std::vector< const LogEntry* > LogQueryEngine::execute( const LogQuery& query ) const {
	if ( query.timeRange.has_value() && query.timeRange->from > query.timeRange->to ) {
		return {};
	}

	const EntryList* indexedCandidates = nullptr;
	if ( query.level.has_value() ) {
		static const EntryList empty;
		const auto it     = byLevel_.find( *query.level );
		indexedCandidates = ( it != byLevel_.end() ) ? &it->second : &empty;
	}

	if ( query.source.has_value() ) {
		static const EntryList empty;
		const auto it                     = bySource_.find( *query.source );
		const EntryList* sourceCandidates = ( it != bySource_.end() ) ? &it->second : &empty;
		if ( indexedCandidates == nullptr || sourceCandidates->size() < indexedCandidates->size() ) {
			indexedCandidates = sourceCandidates;
		}
	}

	EntryList::const_iterator timeBegin = orderedByTimestamp_.begin();
	EntryList::const_iterator timeEnd   = orderedByTimestamp_.end();
	if ( query.timeRange.has_value() ) {
		timeBegin = std::lower_bound( orderedByTimestamp_.begin(), orderedByTimestamp_.end(), query.timeRange->from,
		                              EntryPtrTimestampLess{} );
		timeEnd =
		    std::upper_bound( timeBegin, orderedByTimestamp_.end(), query.timeRange->to, EntryPtrTimestampLess{} );
	}

	const std::size_t timeCandidateCount = static_cast< std::size_t >( std::distance( timeBegin, timeEnd ) );
	const std::size_t indexedCandidateCount =
	    indexedCandidates != nullptr ? indexedCandidates->size() : std::numeric_limits< std::size_t >::max();

	std::vector< const LogEntry* > results;

	if ( indexedCandidates != nullptr && indexedCandidateCount < timeCandidateCount ) {
		results.reserve( indexedCandidates->size() );
		for ( const LogEntry* entry : *indexedCandidates ) {
			if ( matches( *entry, query ) ) {
				results.push_back( entry );
			}
		}
		std::ranges::stable_sort( results, EntryPtrTimestampLess{} );
		return results;
	}

	results.reserve( timeCandidateCount );
	for ( auto it = timeBegin; it != timeEnd; ++it ) {
		if ( matches( **it, query ) ) {
			results.push_back( *it );
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

	if ( query.messageContains.has_value() && !containsCaseInsensitive( entry.message, *query.messageContains ) ) {
		return false;
	}

	if ( query.timeRange.has_value() &&
	     ( entry.timestamp < query.timeRange->from || entry.timestamp > query.timeRange->to ) ) {
		return false;
	}

	return true;
}

}  // namespace task3
