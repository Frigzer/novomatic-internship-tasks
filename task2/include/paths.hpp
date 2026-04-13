#pragma once

#include <filesystem>

namespace task2::paths {

inline std::filesystem::path sourceDir() {
	return std::filesystem::path( TASK2_SOURCE_DIR );
}

inline std::filesystem::path dataDir() {
	return sourceDir() / "data";
}

}  // namespace task2::paths