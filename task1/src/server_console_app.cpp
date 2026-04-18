#include "server_console_app.hpp"

#include "ticket_server.hpp"
#include "ticket_server_host.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace task1 {
namespace {

constexpr std::uint16_t kDefaultPort = 5555;
std::atomic_bool g_keep_running{true};

std::vector<Ticket> makeSampleTickets() {
    return {
        Ticket{.id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt},
        Ticket{.id = 2, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt},
        Ticket{.id = 3, .price = 170, .type = "reduced", .status = TicketStatus::Available, .owner = std::nullopt},
        Ticket{.id = 4, .price = 170, .type = "reduced", .status = TicketStatus::Available, .owner = std::nullopt},
    };
}

std::optional<std::uint16_t> parsePort(const std::string& value) {
    try {
        const auto parsed = std::stoul(value);
        if (parsed > std::numeric_limits<std::uint16_t>::max()) {
            return std::nullopt;
        }
        return static_cast<std::uint16_t>(parsed);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

}  // namespace

ServerConsoleApp::ServerConsoleApp(int argc, char* argv[]) : argc_(argc), argv_(argv), port_(kDefaultPort) {}

int ServerConsoleApp::run() {
    parseArguments();
    if (shouldPrintHelp()) {
        printHelp();
        return 0;
    }

    registerSignalHandlers();
    g_keep_running.store(true);

    CoinInventory cashbox({{200, 1}, {100, 5}, {50, 2}, {20, 10}, {10, 10}});
    TicketServer server(makeSampleTickets(), std::move(cashbox), std::chrono::seconds(60));
    TicketServerHost host(server, port_);

    host.start();
    std::cout << "Ticket server is listening on 127.0.0.1:" << host.port() << '\n';
    std::cout << "Start clients in separate terminals, for example:\n";
    std::cout << "  task1 --host 127.0.0.1 --port " << host.port() << '\n';
    std::cout << "Press Ctrl+C to stop the server.\n";

    while (g_keep_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Stopping server...\n";
    host.stop();
    return 0;
}

void ServerConsoleApp::parseArguments() {
    for (int index = 1; index < argc_; ++index) {
        const std::string_view argument = argv_[index];
        if (argument == "--help" || argument == "-h") {
            show_help_ = true;
            return;
        }

        if (argument == "--port") {
            if (index + 1 >= argc_) {
                throw std::runtime_error("Missing value after --port");
            }

            auto parsed_port = parsePort(argv_[++index]);
            if (!parsed_port.has_value()) {
                throw std::runtime_error("Invalid port value");
            }
            port_ = *parsed_port;
            continue;
        }

        throw std::runtime_error("Unknown argument: " + std::string(argument));
    }
}

bool ServerConsoleApp::shouldPrintHelp() const noexcept {
    return show_help_;
}

void ServerConsoleApp::printHelp() const {
    std::cout << "Usage: task1_server [--port PORT]\n";
}

void ServerConsoleApp::registerSignalHandlers() {
    std::signal(SIGINT, &ServerConsoleApp::onSignal);
}

void ServerConsoleApp::onSignal(int) {
    g_keep_running.store(false);
}

}  // namespace task1
