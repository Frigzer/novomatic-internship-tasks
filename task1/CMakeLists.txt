find_package(asio REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

add_library(task1_lib
    src/change_calculator.cpp
    src/coin_inventory.cpp
    src/network_protocol.cpp
    src/ticket_machine_client.cpp
    src/ticket_server.cpp
    src/ticket_server_host.cpp
)

target_include_directories(task1_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(task1_lib PUBLIC cxx_std_23)

target_link_libraries(task1_lib
    PUBLIC
        asio::asio
        nlohmann_json::nlohmann_json
        Threads::Threads
)

add_executable(task1
    app/main.cpp
)

target_link_libraries(task1 PRIVATE task1_lib)

add_executable(task1_server
    app/server_main.cpp
)

target_link_libraries(task1_server PRIVATE task1_lib)

add_executable(task1_tests
    tests/change_calculator_tests.cpp
    tests/concurrency_tests.cpp
    tests/network_integration_tests.cpp
    tests/ticket_server_tests.cpp
)

target_link_libraries(task1_tests
    PRIVATE
        task1_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(task1_tests)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(task1_lib PRIVATE
            -O0 -g
            -fprofile-instr-generate
            -fcoverage-mapping
        )

        target_compile_options(task1_tests PRIVATE
            -O0 -g
            -fprofile-instr-generate
            -fcoverage-mapping
        )

        target_link_options(task1_tests PRIVATE
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
                    $<TARGET_FILE:task1_tests>
                    -instr-profile=${CMAKE_BINARY_DIR}/coverage/coverage.profdata
                    > ${CMAKE_BINARY_DIR}/coverage/lcov.info
            DEPENDS task1_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            USES_TERMINAL
        )
    else()
        message(FATAL_ERROR "This coverage setup is intended for Clang/LLVM.")
    endif()
endif()
