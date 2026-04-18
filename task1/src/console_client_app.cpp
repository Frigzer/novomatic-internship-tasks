#include "console_client_app.hpp"

#include "coin_inventory.hpp"
#include "ticket_machine_client.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace task1 {
namespace {

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

std::string trim(std::string value) {
    const auto not_space = [](unsigned char character) { return !std::isspace(character); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::vector<std::string> splitTokens(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

}  // namespace

ConsoleClientApp::ConsoleClientApp(int argc, char* argv[]) : argc_(argc), argv_(argv) {}

int ConsoleClientApp::run() {
    parseArguments();
    if (shouldPrintHelp()) {
        printHelp();
        return 0;
    }

    TicketMachineClient client(host_, port_);
    printWelcome();
    commandLoop(client);
    return 0;
}

void ConsoleClientApp::parseArguments() {
    for (int index = 1; index < argc_; ++index) {
        const std::string_view argument = argv_[index];
        if (argument == "--help" || argument == "-h") {
            show_help_ = true;
            return;
        }
        if (argument == "--host") {
            if (index + 1 >= argc_) {
                throw std::runtime_error("Missing value after --host");
            }
            host_ = argv_[++index];
            continue;
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

bool ConsoleClientApp::shouldPrintHelp() const noexcept {
    return show_help_;
}

void ConsoleClientApp::printHelp() const {
    std::cout << "Usage: task1 [--host HOST] [--port PORT]\n";
}

void ConsoleClientApp::printWelcome() const {
    std::cout << "Ticket machine client connected to " << host_ << ':' << port_ << "\n";
    printCommandHelp();
}

void ConsoleClientApp::printCommandHelp() const {
    std::cout << "Available commands:\n"
              << "  help                 Show this help\n"
              << "  list                 Show available tickets\n"
              << "  reserve <type>       Reserve a ticket type\n"
              << "  status               Show current reservation\n"
              << "  buy                  Finalize the current reservation\n"
              << "  cancel               Cancel the current reservation\n"
              << "  quit                 Exit the client\n";
}

void ConsoleClientApp::commandLoop(TicketMachineClient& client) {
    std::string line;
    bool should_exit = false;

    while (!should_exit) {
        std::cout << "task1> " << std::flush;
        if (!std::getline(std::cin, line)) {
            std::cout << "\nInput closed. Exiting client.\n";
            return;
        }

        line = trim(line);
        if (line.empty()) {
            continue;
        }

        try {
            handleCommand(client, line, should_exit);
        } catch (const std::exception& exception) {
            std::cout << "Command failed: " << exception.what() << '\n';
        }
    }
}

void ConsoleClientApp::handleCommand(TicketMachineClient& client, const std::string& line, bool& should_exit) {
    const auto tokens = splitTokens(line);
    const auto& command = tokens.front();

    if (command == "help") {
        printCommandHelp();
        return;
    }
    if (command == "list") {
        printAvailability(client);
        return;
    }
    if (command == "reserve") {
        if (tokens.size() < 2) {
            std::cout << "Usage: reserve <ticket_type>\n";
            return;
        }
        handleReserve(client, tokens[1]);
        return;
    }
    if (command == "status") {
        printStatus();
        return;
    }
    if (command == "buy") {
        handleBuy(client);
        return;
    }
    if (command == "cancel") {
        handleCancel(client);
        return;
    }
    if (command == "quit" || command == "exit") {
        should_exit = true;
        std::cout << "Goodbye.\n";
        return;
    }

    std::cout << "Unknown command: " << command << "\n";
}

void ConsoleClientApp::handleReserve(TicketMachineClient& client, const std::string& ticket_type) {
    if (session_.reservation.has_value()) {
        std::cout << "A reservation is already active. Use 'buy' or 'cancel' first.\n";
        return;
    }

    const auto reservation = client.selectTicket(ticket_type);
    if (!reservation.has_value()) {
        std::cout << "No ticket of type '" << ticket_type << "' is currently available.\n";
        return;
    }

    session_.reservation = reservation;
    std::cout << "Reserved ticket type '" << reservation->ticket_type << "'"
              << " | ticket id: " << reservation->ticket_id
              << " | reservation id: " << reservation->reservation_id
              << " | price: ";
    printMoney(reservation->price);
    std::cout << " PLN\n";
}

void ConsoleClientApp::handleBuy(TicketMachineClient& client) {
    if (!session_.reservation.has_value()) {
        std::cout << "No active reservation. Use 'reserve <type>' first.\n";
        return;
    }

    const auto customer = readCustomerData();
    if (!customer.has_value()) {
        std::cout << "Purchase cancelled before customer data was completed.\n";
        return;
    }

    const auto inserted_coins = readInsertedCoins();
    if (!inserted_coins.has_value()) {
        std::cout << "Purchase cancelled before payment was completed.\n";
        return;
    }

    const auto result = client.buy(session_.reservation->reservation_id, *customer, *inserted_coins);
    if (std::holds_alternative<PurchaseSuccess>(result)) {
        printPurchaseSuccess(std::get<PurchaseSuccess>(result));
        session_.reservation.reset();
        return;
    }

    printPurchaseFailure(std::get<PurchaseFailure>(result));
    session_.reservation.reset();
}

void ConsoleClientApp::handleCancel(TicketMachineClient& client) {
    if (!session_.reservation.has_value()) {
        std::cout << "No active reservation.\n";
        return;
    }

    const auto reservation_id = session_.reservation->reservation_id;
    const bool cancelled = client.cancel(reservation_id);
    session_.reservation.reset();

    if (cancelled) {
        std::cout << "Reservation " << reservation_id << " was cancelled.\n";
    } else {
        std::cout << "Reservation " << reservation_id << " was no longer active.\n";
    }
}

void ConsoleClientApp::printStatus() const {
    if (!session_.reservation.has_value()) {
        std::cout << "No active reservation.\n";
        return;
    }

    std::cout << "Current reservation: ticket type '" << session_.reservation->ticket_type << "'"
              << " | ticket id: " << session_.reservation->ticket_id
              << " | reservation id: " << session_.reservation->reservation_id
              << " | price: ";
    printMoney(session_.reservation->price);
    std::cout << " PLN\n";
}

void ConsoleClientApp::printAvailability(TicketMachineClient& client) {
    const auto availability = client.showAvailableTickets();
    if (availability.empty()) {
        std::cout << "No tickets are currently available.\n";
        return;
    }

    std::cout << "Available tickets:\n";
    for (const auto& item : availability) {
        std::cout << "  - " << item.type << " | price: ";
        printMoney(item.price);
        std::cout << " PLN | available: " << item.available_count << '\n';
    }
}

void ConsoleClientApp::printPurchaseSuccess(const PurchaseSuccess& success) {
    std::cout << "Purchase completed successfully.\n"
              << "  Ticket ID: " << success.ticket_id << '\n'
              << "  Owner: " << success.customer.first_name << ' ' << success.customer.last_name << '\n'
              << "  Paid: ";
    printMoney(success.paid);
    std::cout << " PLN\n  Price: ";
    printMoney(success.price);
    std::cout << " PLN\n  Change: ";
    printMoney(success.change.total);
    std::cout << " PLN\n";

    if (success.change.coins.empty()) {
        std::cout << "  No coins dispensed as change.\n";
        return;
    }

    std::cout << "  Dispensed coins:\n";
    for (const auto& [denomination, count] : success.change.coins) {
        std::cout << "    - " << denomination << " gr x " << count << '\n';
    }
}

void ConsoleClientApp::printPurchaseFailure(const PurchaseFailure& failure) {
    std::cout << "Purchase failed.\n"
              << "  Reason: " << failure.message << '\n';

    if (failure.returned_coins.empty()) {
        std::cout << "  No coins were returned.\n";
        return;
    }

    std::cout << "  Returned coins:\n";
    for (const auto& [denomination, count] : failure.returned_coins) {
        std::cout << "    - " << denomination << " gr x " << count << '\n';
    }
}

void ConsoleClientApp::printMoney(const Money amount) {
    std::cout << amount / 100 << '.' << std::setw(2) << std::setfill('0') << amount % 100;
    std::cout << std::setfill(' ');
}

std::optional<CoinInventory> ConsoleClientApp::readInsertedCoins() {
    std::cout << "Enter inserted coin denominations in grosz, one per line. Type 'done' when finished or 'cancel' to abort.\n";

    std::map<Money, int, std::greater<>> inserted;
    std::string line;
    while (true) {
        std::cout << "coin> " << std::flush;
        if (!std::getline(std::cin, line)) {
            return std::nullopt;
        }

        line = trim(line);
        if (line == "cancel") {
            return std::nullopt;
        }
        if (line == "done") {
            try {
                return CoinInventory(inserted);
            } catch (const std::exception& exception) {
                std::cout << "Invalid coin inventory: " << exception.what() << '\n';
                inserted.clear();
                continue;
            }
        }

        try {
            const auto denomination = static_cast<Money>(std::stoll(line));
            if (denomination <= 0) {
                std::cout << "Coin denomination must be positive.\n";
                continue;
            }
            ++inserted[denomination];
        } catch (const std::exception&) {
            std::cout << "Invalid denomination. Enter the coin value in grosz, for example 500 or 100.\n";
        }
    }
}

std::optional<CustomerData> ConsoleClientApp::readCustomerData() {
    CustomerData customer;

    std::cout << "First name (or 'cancel'): " << std::flush;
    if (!std::getline(std::cin, customer.first_name)) {
        return std::nullopt;
    }
    customer.first_name = trim(customer.first_name);
    if (customer.first_name == "cancel") {
        return std::nullopt;
    }
    if (customer.first_name.empty()) {
        std::cout << "First name cannot be empty.\n";
        return std::nullopt;
    }

    std::cout << "Last name (or 'cancel'): " << std::flush;
    if (!std::getline(std::cin, customer.last_name)) {
        return std::nullopt;
    }
    customer.last_name = trim(customer.last_name);
    if (customer.last_name == "cancel") {
        return std::nullopt;
    }
    if (customer.last_name.empty()) {
        std::cout << "Last name cannot be empty.\n";
        return std::nullopt;
    }

    return customer;
}

}  // namespace task1
