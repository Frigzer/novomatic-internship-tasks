#include "log_store.hpp"

#include "log_parser.hpp"

#include <algorithm>

namespace task3 {

void LogStore::loadFromFile( const std::filesystem::path& path ) {
	entries_ = LogParser::parseFile( path );

	std::ranges::sort( entries_,
	                   []( const LogEntry& lhs, const LogEntry& rhs ) { return lhs.timestamp < rhs.timestamp; } );
}

const std::vector< LogEntry >& LogStore::entries() const noexcept {
	return entries_;
}

std::span< const LogEntry > LogStore::view() const noexcept {
	return entries_;
}

}  // namespace task3
