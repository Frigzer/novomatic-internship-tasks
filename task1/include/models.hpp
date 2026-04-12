#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>


namespace task1 {

using TicketId      = std::uint64_t;
using ReservationId = std::uint64_t;
using Money         = std::int64_t;  // w groszach

enum class TicketStatus { Available, Reserved, Sold };

enum class PurchaseError { ReservationNotFound, ReservationExpired, InsufficientFunds, CannotMakeChange };

struct CustomerData {
	std::string first_name;
	std::string last_name;
};

struct Ticket {
	TicketId id{ 0 };
	Money price{ 0 };
	std::string type;
	TicketStatus status{ TicketStatus::Available };
	std::optional< CustomerData > owner;
};

struct Reservation {
	ReservationId id{ 0 };
	TicketId ticket_id{ 0 };
	std::chrono::system_clock::time_point created_at;
	std::chrono::system_clock::time_point expires_at;
};

struct ChangeResult {
	Money total{ 0 };
	std::map< Money, int, std::greater<> > coins;
};

struct PurchaseSuccess {
	TicketId ticket_id{ 0 };
	Money paid{ 0 };
	Money price{ 0 };
	std::string ticket_type;
	CustomerData customer;
	ChangeResult change;
};

struct PurchaseFailure {
	PurchaseError error;
	std::map< Money, int, std::greater<> > returned_coins;
	std::string message;
};

struct ReservationResult {
	std::string ticket_type;
	ReservationId reservation_id{ 0 };
	TicketId ticket_id{ 0 };
	Money price{ 0 };
};

struct TicketAvailability {
	std::string type;
	Money price{ 0 };
	int available_count{ 0 };
};

}  // namespace task1