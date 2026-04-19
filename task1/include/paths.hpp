#pragma once

#include <filesystem>

namespace task1::paths {

inline const std::filesystem::path sourceDir = std::filesystem::path( TASK1_SOURCE_DIR );
inline const std::filesystem::path dataDir   = sourceDir / "data";

}  // namespace task1::paths
