#include <gtest/gtest.h>

#include "ticket_machine_client.hpp"
#include "ticket_server.hpp"
#include "ticket_server_host.hpp"

#include <atomic>
#include <barrier>
#include <chrono>
#include <thread>
#include <vector>

namespace task1 {
namespace {

using Clock = std::chrono::steady_clock;

std::vector< Ticket > makeSampleTickets() {
	return {
	    Ticket{ .id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt },
	    Ticket{ .id = 2, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt },
	    Ticket{ .id = 3, .price = 170, .type = "reduced", .status = TicketStatus::Available, .owner = std::nullopt } };
}

}  // namespace

TEST( NetworkIntegrationTests, ClientCanReserveAndBuyThroughTcpServer ) {
	auto now = Clock::now();
	TicketServer server( makeSampleTickets(), CoinInventory( { { 200, 1 }, { 100, 5 }, { 50, 2 } } ),
	                     std::chrono::seconds( 60 ), [ &now ] { return now; } );
	TicketServerHost host( server, 0 );
	ASSERT_NO_THROW( host.start() );
	ASSERT_NE( host.port(), 0 );

	TicketMachineClient client( "127.0.0.1", host.port() );
	auto availability = client.showAvailableTickets();
	ASSERT_EQ( availability.size(), 2U );

	auto reservation = client.selectTicket( "normal" );
	ASSERT_TRUE( reservation.has_value() );

	const CustomerData customer{ .first_name = "Jan", .last_name = "Kowalski" };
	const CoinInventory inserted( { { 500, 1 } } );
	auto result = client.buy( reservation->reservation_id, customer, inserted );

	ASSERT_TRUE( std::holds_alternative< PurchaseSuccess >( result ) );
	const auto& success = std::get< PurchaseSuccess >( result );
	EXPECT_EQ( success.ticket_id, 1 );
	EXPECT_EQ( success.change.total, 150 );

	host.stop();
}

TEST( NetworkIntegrationTests, ConcurrentClientsDoNotOversellLastTicket ) {
	auto now = Clock::now();
	TicketServer server(
	    { Ticket{ .id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt } },
	    CoinInventory( { { 200, 1 }, { 100, 5 }, { 50, 2 } } ), std::chrono::seconds( 60 ), [ &now ] { return now; } );
	TicketServerHost host( server, 0 );
	ASSERT_NO_THROW( host.start() );
	ASSERT_NE( host.port(), 0 );

	constexpr int thread_count = 8;
	std::atomic_int success_count{ 0 };
	std::barrier start_line( thread_count );
	std::vector< std::thread > threads;

	for ( int index = 0; index < thread_count; ++index ) {
		threads.emplace_back( [ port = host.port(), &success_count, &start_line ]() {
			start_line.arrive_and_wait();
			TicketMachineClient client( "127.0.0.1", port );
			auto reservation = client.selectTicket( "normal" );
			if ( reservation.has_value() ) {
				++success_count;
			}
		} );
	}

	for ( auto& thread : threads ) {
		thread.join();
	}

	EXPECT_EQ( success_count.load(), 1 );
	host.stop();
}

}  // namespace task1
