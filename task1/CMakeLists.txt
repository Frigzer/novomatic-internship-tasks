add_library(task1_lib
    src/change_calculator.cpp
    src/coin_inventory.cpp
    src/console_client_app.cpp
    src/network_protocol.cpp
    src/server_console_app.cpp
    src/server_seed_data.cpp
    src/ticket_machine_client.cpp
    src/ticket_server.cpp
    src/ticket_server_host.cpp
)

target_include_directories(task1_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(task1_lib PUBLIC cxx_std_23)

target_compile_definitions(task1_lib
    PUBLIC
        TASK1_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

find_package(asio REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)

target_link_libraries(task1_lib
    PUBLIC
        asio::asio
        nlohmann_json::nlohmann_json
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
    tests/server_seed_data_tests.cpp
    tests/ticket_server_tests.cpp
)

target_link_libraries(task1_tests
    PRIVATE
        task1_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(task1_tests)