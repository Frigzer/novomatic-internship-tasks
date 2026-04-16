#include "ticket_machine_client.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

namespace {

void printMoney( task1::Money amount ) {
	std::cout << amount / 100 << "." << std::setw( 2 ) << std::setfill( '0' ) << amount % 100;
}

}  // namespace

int main() {
	using namespace task1;

	const std::vector< Ticket > tickets = {
	    { .id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt },
	    { .id = 2, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt },
	    { .id = 3, .price = 170, .type = "reduced", .status = TicketStatus::Available, .owner = std::nullopt } };

	// Początkowy stan kasetki: 1x2zl, 5x1zl, 2x50gr
	CoinInventory cashbox( { { 200, 1 }, { 100, 5 }, { 50, 2 } } );

	TicketServer server( std::move( tickets ), std::move( cashbox ), std::chrono::seconds( 60 ) );

	TicketMachineClient client( server );

	std::cout << "--- Available tickets ---\n";
	for ( const auto& item : client.showAvailableTickets() ) {
		std::cout << "- " << item.type << " | price: ";
		printMoney( item.price );
		std::cout << " zl | available: " << item.available_count << '\n';
	}

	// Rezerwacja biletów]
	const auto reservation = client.selectTicket( "normal" );
	if ( !reservation.has_value() ) {
		std::cout << "Error: Could not reserve ticket\n";
		return 1;
	}

	std::cout << "\nReserved ticket: " << reservation->ticket_type << " (ID: " << reservation->ticket_id << ")\n";

	// Symulacja zakupu
	const CustomerData customer{ .first_name="Jan", .last_name="Kowalski" };

	// Klient wrzuca 5 zł (500 groszy)
	CoinInventory inserted( { { 500, 1 } } );

	std::cout << "Payment: ";
	printMoney( inserted.total() );
	std::cout << " zl\n";

	auto result = client.buy( reservation->reservation_id, customer, inserted );

	if ( std::holds_alternative< PurchaseSuccess >( result ) ) {
		const auto& success = std::get< PurchaseSuccess >( result );
		std::cout << "\n>>> Purchase successful <<<\n";
		std::cout << "Ticket ID: " << success.ticket_id << '\n';
		std::cout << "Owner: " << success.customer.first_name << ' ' << success.customer.last_name << '\n';

		std::cout << "Change to dispense: ";
		printMoney( success.change.total );
		std::cout << " zl\n";

		for ( const auto& [ denom, count ] : success.change.coins ) {
			std::cout << "  - " << denom << " gr x " << count << '\n';
		}
	} else {
		const auto& failure = std::get< PurchaseFailure >( result );
		std::cout << "\n>>> Purchase failed: " << failure.message << " <<<\n";
	}

	return 0;
}