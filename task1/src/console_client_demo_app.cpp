#include "console_client_demo_app.hpp"

#include "ticket_machine_client.hpp"

#include <iomanip>
#include <iostream>
#include <optional>
#include <vector>

namespace task1 {
namespace {

void printMoney(const Money amount) {
    std::cout << amount / 100 << '.' << std::setw(2) << std::setfill('0') << amount % 100;
}

std::vector<Ticket> makeSampleTickets() {
    return {
        Ticket{.id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt},
        Ticket{.id = 2, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt},
        Ticket{.id = 3, .price = 170, .type = "reduced", .status = TicketStatus::Available, .owner = std::nullopt},
    };
}

CoinInventory makeInitialCashbox() {
    return CoinInventory({{200, 1}, {100, 5}, {50, 2}});
}

void printAvailability(TicketMachineClient& client) {
    std::cout << "--- Available tickets ---\n";
    for (const auto& item : client.showAvailableTickets()) {
        std::cout << "- " << item.type << " | price: ";
        printMoney(item.price);
        std::cout << " PLN | available: " << item.available_count << '\n';
    }
}

void printReservation(const ReservationResult& reservation) {
    std::cout << "\nReserved ticket: " << reservation.ticket_type << " (ticket id: " << reservation.ticket_id
              << ", reservation id: " << reservation.reservation_id << ")\n";
}

void printPurchaseSuccess(const PurchaseSuccess& success) {
    std::cout << "\n>>> Purchase completed successfully <<<\n";
    std::cout << "Ticket ID: " << success.ticket_id << '\n';
    std::cout << "Owner: " << success.customer.first_name << ' ' << success.customer.last_name << '\n';
    std::cout << "Paid: ";
    printMoney(success.paid);
    std::cout << " PLN\n";
    std::cout << "Price: ";
    printMoney(success.price);
    std::cout << " PLN\n";
    std::cout << "Change: ";
    printMoney(success.change.total);
    std::cout << " PLN\n";

    if (success.change.coins.empty()) {
        std::cout << "No change to dispense.\n";
        return;
    }

    std::cout << "Dispensed coins:\n";
    for (const auto& [denomination, count] : success.change.coins) {
        std::cout << "  - " << denomination << " gr x " << count << '\n';
    }
}

void printPurchaseFailure(const PurchaseFailure& failure) {
    std::cout << "\n>>> Purchase failed <<<\n";
    std::cout << "Reason: " << failure.message << '\n';

    if (failure.returned_coins.empty()) {
        std::cout << "No coins were returned.\n";
        return;
    }

    std::cout << "Returned coins:\n";
    for (const auto& [denomination, count] : failure.returned_coins) {
        std::cout << "  - " << denomination << " gr x " << count << '\n';
    }
}

}  // namespace

int ConsoleClientDemoApp::run() {
    TicketServer server(makeSampleTickets(), makeInitialCashbox(), std::chrono::seconds(60));
    TicketMachineClient client(server);

    printAvailability(client);

    const auto reservation = client.selectTicket("normal");
    if (!reservation.has_value()) {
        std::cerr << "Failed to reserve a ticket.\n";
        return 1;
    }

    printReservation(*reservation);

    const CustomerData customer{.first_name = "Jan", .last_name = "Kowalski"};
    const CoinInventory inserted({{500, 1}});

    std::cout << "Payment inserted: ";
    printMoney(inserted.total());
    std::cout << " PLN\n";

    const auto result = client.buy(reservation->reservation_id, customer, inserted);
    if (std::holds_alternative<PurchaseSuccess>(result)) {
        printPurchaseSuccess(std::get<PurchaseSuccess>(result));
        return 0;
    }

    printPurchaseFailure(std::get<PurchaseFailure>(result));
    return 1;
}

}  // namespace task1
