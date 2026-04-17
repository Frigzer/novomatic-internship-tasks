#pragma once

#include <filesystem>
#include <vector>

namespace task3 {

struct FileResolutionResult {
	std::filesystem::path resolved;
	std::vector< std::filesystem::path > attempted;
	bool exists = false;
};

class LogFileResolver {
public:
	[[nodiscard]] std::filesystem::path resolve( const std::filesystem::path& input ) const;
	[[nodiscard]] FileResolutionResult resolveDetailed( const std::filesystem::path& input ) const;

private:
	[[nodiscard]] static bool exists( const std::filesystem::path& candidate ) noexcept;
	[[nodiscard]] static std::filesystem::path normalized( const std::filesystem::path& candidate );
	[[nodiscard]] static bool isPlainFilename( const std::filesystem::path& input ) noexcept;
	static void appendCandidate( std::vector< std::filesystem::path >& candidates, const std::filesystem::path& candidate );
};

}  // namespace task3
