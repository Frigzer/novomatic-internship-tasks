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
)

target_link_libraries(task3_tests
    PRIVATE
        task3_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(task3_tests)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(task3_lib PRIVATE
            -O0 -g
            -fprofile-instr-generate
            -fcoverage-mapping
        )

        target_compile_options(task3_tests PRIVATE
            -O0 -g
            -fprofile-instr-generate
            -fcoverage-mapping
        )

        target_link_options(task3_tests PRIVATE
            -fprofile-instr-generate
        )

        find_program(LLVM_PROFDATA llvm-profdata REQUIRED)
        find_program(LLVM_COV llvm-cov REQUIRED)

        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${LLVM_PROFDATA} merge -sparse
                    ${CMAKE_BINARY_DIR}/coverage/coverage-*.profraw
                    -o ${CMAKE_BINARY_DIR}/coverage/coverage.profdata
            COMMAND ${LLVM_COV} export
                    -format=lcov
                    $<TARGET_FILE:task3_tests>
                    -instr-profile=${CMAKE_BINARY_DIR}/coverage/coverage.profdata
                    > ${CMAKE_BINARY_DIR}/coverage/lcov.info
            DEPENDS task3_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            USES_TERMINAL
        )
    else()
        message(FATAL_ERROR "This coverage setup is intended for Clang/LLVM.")
    endif()
endif()
