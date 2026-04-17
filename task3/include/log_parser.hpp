#pragma once

#include "log_entry.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace task3 {

class LogParser {
public:
	static std::optional< LogEntry > parseLine( const std::string& line );
	static std::vector< LogEntry > parseFile( const std::filesystem::path& path );

private:
	static std::chrono::sys_seconds parseTimestamp( const std::string& text );
};

}  // namespace task3