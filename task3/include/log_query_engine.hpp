#pragma once

#include "log_entry.hpp"
#include "log_query.hpp"

#include <array>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace task3 {

class LogQueryEngine {
public:
    using EntryPtr = const LogEntry*;
    using EntryList = std::vector< EntryPtr >;

    explicit LogQueryEngine( std::span< const LogEntry > entries );

    [[nodiscard]] std::vector< const LogEntry* > execute( const LogQuery& query ) const;

private:
    [[nodiscard]] bool matches( const LogEntry& entry, const LogQuery& query ) const;
    [[nodiscard]] const EntryList* findEntriesBySource( std::string_view source ) const;
    [[nodiscard]] EntryList entriesInTimeRange( const TimeRange& range ) const;

    std::span< const LogEntry > entries_;
    EntryList entriesByTime_;
    std::array< EntryList, 6 > entriesByLevel_;
    std::unordered_map< std::string_view, EntryList > entriesBySource_;
};

}  // namespace task3
