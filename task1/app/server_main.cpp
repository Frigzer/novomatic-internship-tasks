#include "ticket_server.hpp"
#include "ticket_server_host.hpp"

#include <chrono>
#include <iostream>
#include <optional>
#include <vector>

int main() {
	using namespace task1;

	const std::vector< Ticket > tickets = {
	    { .id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt },
	    { .id = 2, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt },
	    { .id = 3, .price = 170, .type = "reduced", .status = TicketStatus::Available, .owner = std::nullopt } };

	CoinInventory cashbox( { { 200, 1 }, { 100, 5 }, { 50, 2 } } );
	TicketServer server( tickets, cashbox, std::chrono::seconds( 60 ) );
	TicketServerHost host( server, 5555 );

	host.start();
	std::cout << "Ticket server listening on 127.0.0.1:" << host.port() << '\n';
	std::cout << "Press ENTER to stop the server...\n";
	std::cin.get();
	host.stop();
	return 0;
}
