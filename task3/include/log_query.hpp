#pragma once

#include "log_entry.hpp"

#include <chrono>
#include <optional>
#include <string>

namespace task3 {

struct TimeRange {
	std::chrono::sys_seconds from;
	std::chrono::sys_seconds to;
};

struct LogQuery {
	std::optional< LogLevel > level;
	std::optional< std::string > source;
	std::optional< std::string > messageContains;
	std::optional< TimeRange > timeRange;
};

}  // namespace task3