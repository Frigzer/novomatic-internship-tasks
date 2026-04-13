add_library(task2_lib
    src/graph.cpp
    src/json_io.cpp
    src/layout_engine.cpp
)

target_include_directories(task2_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(task2_lib PUBLIC cxx_std_23)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(task2_lib PUBLIC nlohmann_json::nlohmann_json)

add_executable(task2
    app/main.cpp
)

target_link_libraries(task2 PRIVATE task2_lib)

find_package(GTest REQUIRED)

add_executable(task2_tests
    tests/json_io_tests.cpp
    tests/layout_engine_tests.cpp
)

target_link_libraries(task2_tests
    PRIVATE
        task2_lib
        GTest::gtest_main
)

target_compile_definitions(task2 PRIVATE
    TASK2_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(task2_tests PRIVATE
    TASK2_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

include(GoogleTest)
gtest_discover_tests(task2_tests)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(task2_lib PRIVATE
            -O0 -g
            -fprofile-instr-generate
            -fcoverage-mapping
        )

        target_compile_options(task2_tests PRIVATE
            -O0 -g
            -fprofile-instr-generate
            -fcoverage-mapping
        )

        target_link_options(task2_tests PRIVATE
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
                    $<TARGET_FILE:task2_tests>
                    -instr-profile=${CMAKE_BINARY_DIR}/coverage/coverage.profdata
                    > ${CMAKE_BINARY_DIR}/coverage/lcov.info
            DEPENDS task2_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            USES_TERMINAL
        )
    else()
        message(FATAL_ERROR "This coverage setup is intended for Clang/LLVM.")
    endif()
endif()