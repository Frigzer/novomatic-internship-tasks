#pragma once

#include <filesystem>
#include <vector>

namespace task3::LogFileResolver {

struct FileResolutionResult {
	std::filesystem::path resolved;
	std::vector< std::filesystem::path > attempted;
	bool exists = false;
};

[[nodiscard]] std::filesystem::path resolve( const std::filesystem::path& input );
[[nodiscard]] FileResolutionResult resolveDetailed( const std::filesystem::path& input );

}  // namespace task3::LogFileResolver
