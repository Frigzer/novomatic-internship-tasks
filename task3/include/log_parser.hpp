#pragma once

#include "log_entry.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace task3 {

class LogParser {
public:
    [[nodiscard]] static std::optional< LogEntry > parseLine( const std::string& line );
    [[nodiscard]] static std::vector< LogEntry > parseFile( const std::filesystem::path& path );

private:
    [[nodiscard]] static std::chrono::sys_seconds parseTimestamp( std::string_view text );
};

}  // namespace task3
