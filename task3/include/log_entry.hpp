#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

namespace task3 {

enum class LogLevel : std::uint8_t { Trace, Debug, Info, Warn, Error, Fatal };

struct LogEntry {
    std::chrono::sys_seconds timestamp;
    LogLevel level;
    std::string source;
    std::string message;
    std::string raw_line;
};

[[nodiscard]] std::string_view toString( LogLevel level ) noexcept;
[[nodiscard]] LogLevel parseLogLevel( std::string_view text );

}  // namespace task3
