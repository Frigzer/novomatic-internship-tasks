#pragma once

#include <filesystem>

namespace task3 {

class LogFileResolver {
public:
    [[nodiscard]] std::filesystem::path resolve( const std::filesystem::path& input ) const;

private:
    [[nodiscard]] static bool exists( const std::filesystem::path& candidate ) noexcept;
    [[nodiscard]] static std::filesystem::path normalized( const std::filesystem::path& candidate );
    [[nodiscard]] static bool isPlainFilename( const std::filesystem::path& input ) noexcept;
};

}  // namespace task3
