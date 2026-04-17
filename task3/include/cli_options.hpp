#pragma once

#include "log_query.hpp"

#include <filesystem>

namespace task3 {

struct CliOptions {
	std::filesystem::path logFile;
	LogQuery query;
	bool countOnly = false;
};

}  // namespace task3
