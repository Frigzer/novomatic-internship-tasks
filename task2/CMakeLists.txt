cmake_minimum_required(VERSION 3.25)
project(task2 LANGUAGES CXX)

add_library(task2_lib
    src/blueprint_file_manager.cpp
    src/blueprint_viewer_app.cpp
    src/blueprint_viewer_panel.cpp
    src/graph.cpp
    src/graph_renderer.cpp
    src/graph_view_controller.cpp
    src/imgui_sfml_context.cpp
    src/json_io.cpp
    src/layout_engine.cpp
)

target_include_directories(task2_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(task2_lib PUBLIC cxx_std_23)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(SFML CONFIG REQUIRED COMPONENTS Graphics Window System)
find_package(imgui-sfml CONFIG REQUIRED)

target_link_libraries(task2_lib 
    PUBLIC 
        nlohmann_json::nlohmann_json
        SFML::Graphics
        SFML::Window
        SFML::System
        ImGui-SFML::ImGui-SFML
)

target_compile_definitions(task2_lib
    PUBLIC
        TASK2_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(task2
    app/main.cpp
)

target_link_libraries(task2 PRIVATE task2_lib)

find_package(GTest REQUIRED)

add_executable(task2_tests
    tests/graph_renderer_tests.cpp
    tests/graph_tests.cpp
    tests/integration_tests.cpp
    tests/json_io_tests.cpp
    tests/layout_engine_tests.cpp
)

target_link_libraries(task2_tests
    PRIVATE
        task2_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(task2_tests)