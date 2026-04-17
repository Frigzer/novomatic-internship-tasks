#pragma once

#include "log_entry.hpp"

#include <filesystem>
#include <vector>

namespace task3 {

class LogStore {
public:
    void loadFromFile( const std::filesystem::path& path );

    [[nodiscard]] const std::vector< LogEntry >& entries() const noexcept;

private:
    std::vector< LogEntry > entries_;
};

}  // namespace task3
