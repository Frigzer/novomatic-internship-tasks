#pragma once

#include "log_entry.hpp"

#include <filesystem>
#include <vector>

namespace task3 {

class LogStore {
public:
	void loadFromFile( const std::filesystem::path& path );

	const std::vector< LogEntry >& entries() const;

private:
	std::vector< LogEntry > entries_;
};

}  // namespace task3