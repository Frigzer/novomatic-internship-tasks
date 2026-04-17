#pragma once

#include "log_entry.hpp"
#include "log_query.hpp"

#include <span>
#include <vector>

namespace task3 {

class LogQueryEngine {
public:
	explicit LogQueryEngine( std::span< const LogEntry > entries );

	std::vector< const LogEntry* > execute( const LogQuery& query ) const;

private:
	bool matches( const LogEntry& entry, const LogQuery& query ) const;

	std::span< const LogEntry > entries_;
};

}  // namespace task3