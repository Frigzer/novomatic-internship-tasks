#pragma once

#include <filesystem>

namespace task2::paths {

inline const std::filesystem::path sourceDir = std::filesystem::path( TASK2_SOURCE_DIR );
inline const std::filesystem::path dataDir   = sourceDir / "data";

}  // namespace task2::paths