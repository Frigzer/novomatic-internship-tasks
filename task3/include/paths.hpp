#pragma once

#include <filesystem>

namespace task3::paths {

inline const std::filesystem::path sourceDir = std::filesystem::path( TASK3_SOURCE_DIR );
inline const std::filesystem::path dataDir   = sourceDir / "data";

}  // namespace task3::paths
