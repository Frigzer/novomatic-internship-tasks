cmake_minimum_required(VERSION 3.15.0)
project(novomatic-internship-tasks VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_COVERAGE "Enable LLVM source-based coverage" ON)

enable_testing()

# add_subdirectory(task1)
add_subdirectory(task2)
