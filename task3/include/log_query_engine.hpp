#pragma once

#include "log_entry.hpp"
#include "log_query.hpp"

#include <span>
#include <unordered_map>
#include <vector>

namespace task3 {

class LogQueryEngine {
public:
	explicit LogQueryEngine( std::span< const LogEntry > entries );

	[[nodiscard]] std::vector< const LogEntry* > execute( const LogQuery& query ) const;

private:
	using EntryPtr  = const LogEntry*;
	using EntryList = std::vector< EntryPtr >;

	[[nodiscard]] bool matches( const LogEntry& entry, const LogQuery& query ) const;

	std::span< const LogEntry > entries_;
	EntryList orderedByTimestamp_;
	std::unordered_map< LogLevel, EntryList > byLevel_;
	std::unordered_map< std::string_view, EntryList > bySource_;
};

}  // namespace task3
