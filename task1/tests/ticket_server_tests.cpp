#include <gtest/gtest.h>

#include "ticket_server.hpp"

namespace task1 {
namespace {

using Clock = std::chrono::system_clock;

std::vector< Ticket > makeSampleTickets() {
	return { Ticket{ static_cast< Money >( 1 ), 350, "normal", TicketStatus::Available, std::nullopt },
	         Ticket{ static_cast< TicketId >( 2 ), 350, "normal", TicketStatus::Available, std::nullopt },
	         Ticket{ static_cast< TicketId >( 3 ), 170, "reduced", TicketStatus::Available, std::nullopt } };
}

}  // namespace

TEST( TicketServerTests, ReserveTicketReducesAvailability ) {
	auto now = Clock::now();
	TicketServer server( makeSampleTickets(), CoinInventory( { { 200, 1 }, { 100, 5 }, { 50, 2 } } ),
	                     std::chrono::seconds( 60 ), [ &now ] { return now; } );

	auto before      = server.getAvailableTickets();
	auto reservation = server.reserveTicket( "normal" );
	auto after       = server.getAvailableTickets();

	ASSERT_TRUE( reservation.has_value() );
	ASSERT_EQ( before.size(), 2U );
	ASSERT_EQ( after.size(), 2U );

	auto findNormalCount = []( const std::vector< TicketAvailability >& items ) {
		for ( const auto& item : items ) {
			if ( item.type == "normal" ) {
				return item.available_count;
			}
		}
		return -1;
	};

	EXPECT_EQ( findNormalCount( before ), 2 );
	EXPECT_EQ( findNormalCount( after ), 1 );
}

TEST( TicketServerTests, CancelReservationReturnsTicketToPool ) {
	auto now = Clock::now();
	TicketServer server( makeSampleTickets(), CoinInventory( { { 200, 1 }, { 100, 5 }, { 50, 2 } } ),
	                     std::chrono::seconds( 60 ), [ &now ] { return now; } );

	auto reservation = server.reserveTicket( "reduced" );
	ASSERT_TRUE( reservation.has_value() );

	EXPECT_TRUE( server.cancelReservation( reservation->reservation_id ) );

	auto available = server.getAvailableTickets();

	bool foundReduced = false;
	for ( const auto& item : available ) {
		if ( item.type == "reduced" ) {
			foundReduced = true;
			EXPECT_EQ( item.available_count, 1 );
		}
	}

	EXPECT_TRUE( foundReduced );
}

TEST( TicketServerTests, TimeoutReleasesReservation ) {
	auto now = Clock::now();
	TicketServer server( { Ticket{ 1, 350, "normal", TicketStatus::Available, std::nullopt } },
	                     CoinInventory( { { 200, 1 }, { 100, 5 }, { 50, 2 } } ), std::chrono::seconds( 60 ),
	                     [ &now ] { return now; } );

	auto reservation = server.reserveTicket( "normal" );
	ASSERT_TRUE( reservation.has_value() );

	now += std::chrono::seconds( 61 );

	auto available = server.getAvailableTickets();

	ASSERT_EQ( available.size(), 1U );
	EXPECT_EQ( available[ 0 ].type, "normal" );
	EXPECT_EQ( available[ 0 ].available_count, 1 );
}

TEST( TicketServerTests, SuccessfulPurchaseAssignsTicketAndReturnsChange ) {
	auto now = Clock::now();
	TicketServer server( { Ticket{ 1, 350, "normal", TicketStatus::Available, std::nullopt } },
	                     CoinInventory( { { 200, 1 }, { 100, 5 }, { 50, 2 } } ), std::chrono::seconds( 60 ),
	                     [ &now ] { return now; } );

	auto reservation = server.reserveTicket( "normal" );
	ASSERT_TRUE( reservation.has_value() );

	CoinInventory inserted( { { 500, 1 } } );
	CustomerData customer{ "Jan", "Kowalski" };

	auto result = server.finalizePurchase( reservation->reservation_id, customer, inserted );

	ASSERT_TRUE( std::holds_alternative< PurchaseSuccess >( result ) );
	const auto& success = std::get< PurchaseSuccess >( result );

	EXPECT_EQ( success.ticket_id, 1 );
	EXPECT_EQ( success.customer.first_name, "Jan" );
	EXPECT_EQ( success.customer.last_name, "Kowalski" );
	EXPECT_EQ( success.paid, 500 );
	EXPECT_EQ( success.price, 350 );
	EXPECT_EQ( success.change.total, 150 );
	EXPECT_EQ( success.change.coins.at( 100 ), 1 );
	EXPECT_EQ( success.change.coins.at( 50 ), 1 );
}

TEST( TicketServerTests, InsufficientFundsReturnsInsertedCoinsAndReleasesReservation ) {
	auto now = Clock::now();
	TicketServer server( { Ticket{ 1, 350, "normal", TicketStatus::Available, std::nullopt } },
	                     CoinInventory( { { 200, 1 }, { 100, 5 }, { 50, 2 } } ), std::chrono::seconds( 60 ),
	                     [ &now ] { return now; } );

	auto reservation = server.reserveTicket( "normal" );
	ASSERT_TRUE( reservation.has_value() );

	CoinInventory inserted( { { 200, 1 }, { 100, 1 } } );
	CustomerData customer{ "Jan", "Kowalski" };

	auto result = server.finalizePurchase( reservation->reservation_id, customer, inserted );

	ASSERT_TRUE( std::holds_alternative< PurchaseFailure >( result ) );
	const auto& failure = std::get< PurchaseFailure >( result );

	EXPECT_EQ( failure.error, PurchaseError::InsufficientFunds );
	EXPECT_EQ( failure.returned_coins.at( 200 ), 1 );
	EXPECT_EQ( failure.returned_coins.at( 100 ), 1 );

	auto available = server.getAvailableTickets();
	ASSERT_EQ( available.size(), 1U );
	EXPECT_EQ( available[ 0 ].available_count, 1 );
}

TEST( TicketServerTests, CannotMakeChangeReturnsInsertedCoinsAndReleasesReservation ) {
	auto now = Clock::now();
	TicketServer server( { Ticket{ 1, 280, "special", TicketStatus::Available, std::nullopt } },
	                     CoinInventory( { { 200, 10 } } ), std::chrono::seconds( 60 ), [ &now ] { return now; } );

	auto reservation = server.reserveTicket( "special" );
	ASSERT_TRUE( reservation.has_value() );

	CoinInventory inserted( { { 500, 1 } } );
	CustomerData customer{ "Jan", "Kowalski" };

	auto result = server.finalizePurchase( reservation->reservation_id, customer, inserted );

	ASSERT_TRUE( std::holds_alternative< PurchaseFailure >( result ) );
	const auto& failure = std::get< PurchaseFailure >( result );

	EXPECT_EQ( failure.error, PurchaseError::CannotMakeChange );
	EXPECT_EQ( failure.returned_coins.at( 500 ), 1 );

	auto available = server.getAvailableTickets();
	ASSERT_EQ( available.size(), 1U );
	EXPECT_EQ( available[ 0 ].available_count, 1 );
}

}  // namespace task1