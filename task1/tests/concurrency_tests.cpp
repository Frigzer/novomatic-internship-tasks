#include <gtest/gtest.h>

#include "ticket_server.hpp"

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

namespace task1 {
namespace {

using Clock = std::chrono::system_clock;

}  // namespace

TEST( ConcurrencyTests, OnlyOneClientCanReserveLastTicket ) {
	auto now = Clock::now();

	TicketServer server(
	    { Ticket{ .id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt } },
	    CoinInventory( { { 200, 1 }, { 100, 5 }, { 50, 2 } } ), std::chrono::seconds( 60 ), [ &now ] { return now; } );

	constexpr int thread_count = 10;
	std::atomic< int > success_count{ 0 };
	std::vector< std::thread > threads;

	for ( int i = 0; i < thread_count; ++i ) {
		threads.emplace_back( [ &server, &success_count ]() {
			auto reservation = server.reserveTicket( "normal" );
			if ( reservation.has_value() ) {
				++success_count;
			}
		} );
	}

	for ( auto& thread : threads ) {
		thread.join();
	}

	EXPECT_EQ( success_count.load(), 1 );

	const auto available = server.getAvailableTickets();
	ASSERT_EQ( available.size(), 1U );
	EXPECT_EQ( available[ 0 ].available_count, 0 );
}

TEST( ConcurrencyTests, NumberOfSuccessfulReservationsMatchesTicketPool ) {
	auto now = Clock::now();

	std::vector< Ticket > tickets;
	for ( int i = 1; i <= 5; ++i ) {
		tickets.push_back( Ticket{ .id     = static_cast< TicketId >( i ),
		                           .price  = 350,
		                           .type   = "normal",
		                           .status = TicketStatus::Available,
		                           .owner  = std::nullopt } );
	}

	TicketServer server( std::move( tickets ), CoinInventory( { { 200, 2 }, { 100, 10 }, { 50, 10 } } ),
	                     std::chrono::seconds( 60 ), [ &now ] { return now; } );

	constexpr int thread_count = 20;
	std::atomic< int > success_count{ 0 };
	std::vector< std::thread > threads;

	for ( int i = 0; i < thread_count; ++i ) {
		threads.emplace_back( [ &server, &success_count ]() {
			auto reservation = server.reserveTicket( "normal" );
			if ( reservation.has_value() ) {
				++success_count;
			}
		} );
	}

	for ( auto& thread : threads ) {
		thread.join();
	}

	EXPECT_EQ( success_count.load(), 5 );

	const auto available = server.getAvailableTickets();
	ASSERT_EQ( available.size(), 1U );
	EXPECT_EQ( available[ 0 ].available_count, 0 );
}

TEST( ConcurrencyTests, ConcurrentPurchasesDoNotOversellTickets ) {
	auto now = Clock::now();

	std::vector< Ticket > tickets;
	for ( int i = 1; i <= 3; ++i ) {
		tickets.push_back( Ticket{ .id     = static_cast< TicketId >( i ),
		                           .price  = 350,
		                           .type   = "normal",
		                           .status = TicketStatus::Available,
		                           .owner  = std::nullopt } );
	}

	TicketServer server( std::move( tickets ), CoinInventory( { { 200, 3 }, { 100, 10 }, { 50, 10 } } ),
	                     std::chrono::seconds( 60 ), [ &now ] { return now; } );

	constexpr int thread_count = 10;
	std::atomic< int > success_count{ 0 };
	std::atomic< int > failure_count{ 0 };
	std::vector< std::thread > threads;

	for ( int i = 0; i < thread_count; ++i ) {
		threads.emplace_back( [ &server, &success_count, &failure_count, i ]() {
			auto reservation = server.reserveTicket( "normal" );
			if ( !reservation.has_value() ) {
				++failure_count;
				return;
			}

			CoinInventory inserted( { { 500, 1 } } );
			CustomerData customer{ .first_name = "User", .last_name = std::to_string( i ) };

			auto result = server.finalizePurchase( reservation->reservation_id, customer, inserted );

			if ( std::holds_alternative< PurchaseSuccess >( result ) ) {
				++success_count;
			} else {
				++failure_count;
			}
		} );
	}

	for ( auto& thread : threads ) {
		thread.join();
	}

	EXPECT_EQ( success_count.load(), 3 );
	EXPECT_EQ( failure_count.load(), 7 );

	const auto available = server.getAvailableTickets();
	ASSERT_EQ( available.size(), 1U );
	EXPECT_EQ( available[ 0 ].available_count, 0 );
}

}  // namespace task1