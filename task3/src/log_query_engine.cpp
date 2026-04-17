#include "log_query_engine.hpp"

#include <algorithm>
#include <ranges>

namespace task3 {
namespace {

[[nodiscard]] constexpr std::size_t toIndex( LogLevel level ) noexcept {
    return static_cast< std::size_t >( level );
}

struct EntryTimestampLess {
    bool operator()( LogQueryEngine::EntryPtr lhs, const std::chrono::sys_seconds& rhs ) const noexcept {
        return lhs->timestamp < rhs;
    }

    bool operator()( const std::chrono::sys_seconds& lhs, LogQueryEngine::EntryPtr rhs ) const noexcept {
        return lhs < rhs->timestamp;
    }
};

}  // namespace

LogQueryEngine::LogQueryEngine( std::span< const LogEntry > entries ) : entries_( entries ) {
    entriesByTime_.reserve( entries_.size() );
    for ( const LogEntry& entry : entries_ ) {
        entriesByTime_.push_back( &entry );
    }

    std::ranges::sort( entriesByTime_, {}, &LogEntry::timestamp );

    for ( EntryPtr entry : entriesByTime_ ) {
        entriesByLevel_[ toIndex( entry->level ) ].push_back( entry );
        entriesBySource_[ entry->source ].push_back( entry );
    }
}

std::vector< const LogEntry* > LogQueryEngine::execute( const LogQuery& query ) const {
    if ( query.timeRange.has_value() && query.timeRange->from > query.timeRange->to ) {
        return {};
    }

    const EntryList* candidateSet = &entriesByTime_;
    EntryList timeFilteredEntries;

    if ( query.timeRange.has_value() ) {
        timeFilteredEntries = entriesInTimeRange( *query.timeRange );
        candidateSet        = &timeFilteredEntries;
    }

    if ( query.level.has_value() ) {
        const EntryList& levelEntries = entriesByLevel_[ toIndex( *query.level ) ];
        if ( levelEntries.size() < candidateSet->size() ) {
            candidateSet = &levelEntries;
        }
    }

    if ( query.source.has_value() ) {
        const EntryList* sourceEntries = findEntriesBySource( *query.source );
        if ( sourceEntries == nullptr ) {
            return {};
        }

        if ( sourceEntries->size() < candidateSet->size() ) {
            candidateSet = sourceEntries;
        }
    }

    std::vector< const LogEntry* > results;
    results.reserve( candidateSet->size() );

    for ( EntryPtr entry : *candidateSet ) {
        if ( matches( *entry, query ) ) {
            results.push_back( entry );
        }
    }

    if ( !std::ranges::is_sorted( results, {}, &LogEntry::timestamp ) ) {
        std::ranges::sort( results, {}, &LogEntry::timestamp );
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

    if ( query.messageContains.has_value() && entry.message.find( *query.messageContains ) == std::string::npos ) {
        return false;
    }

    if ( query.timeRange.has_value() &&
         ( entry.timestamp < query.timeRange->from || entry.timestamp > query.timeRange->to ) ) {
        return false;
    }

    return true;
}

const LogQueryEngine::EntryList* LogQueryEngine::findEntriesBySource( std::string_view source ) const {
    const auto it = entriesBySource_.find( source );
    if ( it == entriesBySource_.end() ) {
        return nullptr;
    }

    return &it->second;
}

LogQueryEngine::EntryList LogQueryEngine::entriesInTimeRange( const TimeRange& range ) const {
    const auto begin = std::lower_bound( entriesByTime_.begin(), entriesByTime_.end(), range.from, EntryTimestampLess{} );
    const auto end   = std::upper_bound( begin, entriesByTime_.end(), range.to, EntryTimestampLess{} );

    return EntryList( begin, end );
}

}  // namespace task3
