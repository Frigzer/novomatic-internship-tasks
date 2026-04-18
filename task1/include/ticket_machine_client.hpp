#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace task1 {

class TicketServer;

class TicketMachineClient {
public:
	explicit TicketMachineClient( TicketServer& server );
	TicketMachineClient( std::string host, std::uint16_t port );
	~TicketMachineClient();

	TicketMachineClient( const TicketMachineClient& )            = delete;
	TicketMachineClient& operator=( const TicketMachineClient& ) = delete;
	TicketMachineClient( TicketMachineClient&& ) noexcept;
	TicketMachineClient& operator=( TicketMachineClient&& ) noexcept;

	std::vector< TicketAvailability > showAvailableTickets();
	std::optional< ReservationResult > selectTicket( std::string_view ticket_type );
	bool cancel( ReservationId reservation_id );
	std::variant< PurchaseSuccess, PurchaseFailure > buy( ReservationId reservation_id, const CustomerData& customer,
	                                                      const CoinInventory& inserted_coins );

private:
	class Impl;
	std::unique_ptr< Impl > impl_;
	TicketServer* local_server_{ nullptr };
};

}  // namespace task1
