#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <functional>
#include <mutex>
#include <variant>
#include <vector>
#include <chrono>

namespace task1 {

class TicketServer {
public:
    using TimePoint = std::chrono::system_clock::time_point;
    using ClockFn = std::function<TimePoint()>;

    TicketServer(std::vector<Ticket> tickets,
                 CoinInventory initial_cashbox,
                 std::chrono::seconds reservation_timeout,
                 ClockFn clock = [] { return std::chrono::system_clock::now(); });

    std::vector<TicketAvailability> getAvailableTickets();

    std::optional<ReservationResult> reserveTicket(const std::string& ticket_type);

    bool cancelReservation(ReservationId reservation_id);

    std::variant<PurchaseSuccess, PurchaseFailure> finalizePurchase(
        ReservationId reservation_id,
        const CustomerData& customer,
        const CoinInventory& inserted_coins);

private:
    void cleanupExpiredReservations();

    Ticket* findAvailableTicketByType(const std::string& ticket_type);
    Ticket* findTicketById(TicketId ticket_id);
    Reservation* findReservationById(ReservationId reservation_id);
    
    void removeReservation(ReservationId reservation_id);

private:
    mutable std::mutex mutex_;

    std::vector<Ticket> tickets_;
    CoinInventory cashbox_;
    std::vector<Reservation> reservations_;

    std::chrono::seconds reservation_timeout_;
    ClockFn clock_;

    ReservationId next_reservation_id_{1};
};

} // namespace task1