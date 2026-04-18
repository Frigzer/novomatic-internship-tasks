#pragma once

#include "ticket_server.hpp"
#include <string_view>

namespace task1 {

class TicketMachineClient {
public:
	explicit TicketMachineClient( TicketServer& server );

	std::vector< TicketAvailability > showAvailableTickets();

	std::optional< ReservationResult > selectTicket( std::string_view ticket_type );

	bool cancel( ReservationId reservation_id );

	std::variant< PurchaseSuccess, PurchaseFailure > buy( ReservationId reservation_id, const CustomerData& customer,
	                                                      const CoinInventory& inserted_coins );

private:
	TicketServer& server_;
};

}  // namespace task1