add_library(task3_lib
    src/cli_parser.cpp
    src/log_console_app.cpp
    src/log_file_resolver.cpp
    src/log_parser.cpp
    src/log_store.cpp
    src/log_query_engine.cpp
)

target_include_directories(task3_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(task3_lib PUBLIC cxx_std_23)

target_compile_definitions(task3_lib
    PUBLIC
        TASK3_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(task3
    app/main.cpp
)

target_link_libraries(task3 PRIVATE task3_lib)

set_target_properties(task3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/task3"
)

add_custom_command(TARGET task3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/data"
            "${CMAKE_BINARY_DIR}/task3/data"
)

find_package(GTest REQUIRED)

add_executable(task3_tests
    tests/cli_parser_tests.cpp
    tests/log_file_resolver_tests.cpp
    tests/log_parser_tests.cpp
    tests/log_query_engine_tests.cpp
    tests/log_store_tests.cpp
    tests/log_console_app_tests.cpp
)

target_link_libraries(task3_tests
    PRIVATE
        task3_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(task3_tests)