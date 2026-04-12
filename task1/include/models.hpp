#pragma once

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace task1 {

using TicketId = int;
using ReservationId = int;
using Money = int; // w groszach

enum class TicketStatus {
    Available,
    Reserved,
    Sold
};

enum class PurchaseError {
    ReservationNotFound,
    ReservationExpired,
    InsufficientFunds,
    CannotMakeChange
};

struct CustomerData {
    std::string first_name;
    std::string last_name;
};

struct Ticket {
    TicketId id{};
    std::string type;
    Money price{};
    TicketStatus status{TicketStatus::Available};
    std::optional<CustomerData> owner;
};

struct Reservation {
    ReservationId id{};
    TicketId ticket_id{};
    std::chrono::steady_clock::time_point created_at;
    std::chrono::steady_clock::time_point expires_at;
};

struct ChangeResult {
    Money total{};
    std::map<Money, int, std::greater<>> coins;
};

struct PurchaseSuccess {
    TicketId ticket_id{};
    std::string ticket_type;
    CustomerData customer;
    Money paid{};
    Money price{};
    ChangeResult change;
};

struct PurchaseFailure {
    PurchaseError error;
    std::string message;
    std::map<Money, int, std::greater<>> returned_coins;
};

struct ReservationResult {
    ReservationId reservation_id{};
    TicketId ticket_id{};
    std::string ticket_type;
    Money price{};
};

struct TicketAvailability {
    std::string type;
    Money price{};
    int available_count{};
};

} // namespace task1