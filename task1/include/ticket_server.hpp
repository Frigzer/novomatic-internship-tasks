#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <chrono>
#include <expected>
#include <functional>
#include <mutex>
#include <unordered_set>
#include <variant>
#include <vector>

namespace task1 {

class TicketServer {
public:
	using TimePoint = std::chrono::steady_clock::time_point;
	using ClockFn   = std::function< TimePoint() >;

	TicketServer(
	    std::vector< Ticket > tickets, CoinInventory initial_cashbox, std::chrono::seconds reservation_timeout,
	    ClockFn clock = [] { return std::chrono::steady_clock::now(); } );

	std::vector< TicketAvailability > getAvailableTickets();

	std::optional< ReservationResult > reserveTicket( const std::string& ticket_type );

	bool cancelReservation( ReservationId reservation_id );

	std::variant< PurchaseSuccess, PurchaseFailure >
	finalizePurchase( ReservationId reservation_id, const CustomerData& customer, const CoinInventory& inserted_coins );

private:
	void cleanupExpiredReservations();

	std::expected< Ticket*, std::monostate > findAvailableTicketByType( const std::string& ticket_type );
	Ticket* findTicketById( TicketId ticket_id );
	Reservation* findReservationById( ReservationId reservation_id );
	bool isReservationExpired( ReservationId reservation_id ) const;

	void removeReservation( ReservationId reservation_id );

	mutable std::mutex mutex_;

	std::vector< Ticket > tickets_;
	CoinInventory cashbox_;
	std::vector< Reservation > reservations_;
	std::unordered_set< ReservationId > expired_reservation_ids_;

	std::chrono::seconds reservation_timeout_;
	ClockFn clock_;

	ReservationId next_reservation_id_{ 1 };
};

}  // namespace task1
