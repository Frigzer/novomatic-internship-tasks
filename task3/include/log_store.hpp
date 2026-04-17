#pragma once

#include "log_entry.hpp"

#include <filesystem>
#include <span>
#include <vector>

namespace task3 {

class LogStore {
public:
	void loadFromFile( const std::filesystem::path& path );

	[[nodiscard]] const std::vector< LogEntry >& entries() const noexcept;
	[[nodiscard]] std::span< const LogEntry > view() const noexcept;

private:
	std::vector< LogEntry > entries_;
};

}  // namespace task3
