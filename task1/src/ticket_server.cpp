#include "ticket_server.hpp"
#include "change_calculator.hpp"

#include <algorithm>
#include <unordered_map>

namespace task1 {

TicketServer::TicketServer( std::vector< Ticket > tickets, CoinInventory initial_cashbox,
                            std::chrono::seconds reservation_timeout, ClockFn clock )
    : tickets_( std::move( tickets ) ),
      cashbox_( std::move( initial_cashbox ) ),
      reservation_timeout_( reservation_timeout ),
      clock_( std::move( clock ) ) {}

std::vector< TicketAvailability > TicketServer::getAvailableTickets() {
	std::scoped_lock lock( mutex_ );
	cleanupExpiredReservations();

	std::unordered_map< std::string, TicketAvailability > aggregated;

	for ( const auto& ticket : tickets_ ) {
		auto& entry = aggregated[ ticket.type ];
		entry.type  = ticket.type;
		entry.price = ticket.price;

		if ( ticket.status == TicketStatus::Available ) {
			entry.available_count++;
		}
	}

	std::vector< TicketAvailability > result;
	result.reserve( aggregated.size() );
	for ( auto& [ _, availability ] : aggregated ) {
		result.push_back( availability );
	}

	std::sort( result.begin(), result.end(), []( const auto& lhs, const auto& rhs ) {
		if ( lhs.type != rhs.type ) return lhs.type < rhs.type;
		return lhs.price < rhs.price;
	} );

	return result;
}

std::optional< ReservationResult > TicketServer::reserveTicket( const std::string& ticket_type ) {
	std::scoped_lock lock( mutex_ );
	cleanupExpiredReservations();

	const auto result = findAvailableTicketByType( ticket_type );
	if ( !result ) return std::nullopt;

	Ticket& ticket = *result.value();
	ticket.status  = TicketStatus::Reserved;

	const auto now = clock_();
	Reservation res{ .id         = next_reservation_id_++,
	                 .ticket_id  = ticket.id,
	                 .created_at = now,
	                 .expires_at = now + reservation_timeout_ };

	reservations_.push_back( res );

	return ReservationResult{
	    .ticket_type = ticket.type, .reservation_id = res.id, .ticket_id = ticket.id, .price = ticket.price };
}

bool TicketServer::cancelReservation( ReservationId reservation_id ) {
	std::scoped_lock lock( mutex_ );
	cleanupExpiredReservations();

	auto* res = findReservationById( reservation_id );
	if ( !res ) {
		return false;
	}

	Ticket* ticket = findTicketById( res->ticket_id );
	if ( ticket && ticket->status == TicketStatus::Reserved ) {
		ticket->status = TicketStatus::Available;
	}

	removeReservation( reservation_id );
	return true;
}

std::variant< PurchaseSuccess, PurchaseFailure > TicketServer::finalizePurchase( ReservationId reservation_id,
                                                                                 const CustomerData& customer,
                                                                                 const CoinInventory& inserted_coins ) {
	std::scoped_lock lock( mutex_ );
	cleanupExpiredReservations();

	Reservation* res = findReservationById( reservation_id );
	Ticket* ticket   = res ? findTicketById( res->ticket_id ) : nullptr;

	auto fail = [ & ]( PurchaseError err, std::string msg ) {
		if ( ticket && ticket->status == TicketStatus::Reserved ) {
			ticket->status = TicketStatus::Available;
		}
		removeReservation( reservation_id );
		return PurchaseFailure{
		    .error = err, .returned_coins = inserted_coins.getCoins(), .message = std::move( msg ) };
	};

	if ( !res || !ticket || ticket->status != TicketStatus::Reserved ) {
		if ( isReservationExpired( reservation_id ) ) {
			return fail( PurchaseError::ReservationExpired, "Reservation expired" );
		}
		return fail( PurchaseError::ReservationNotFound, "Reservation not found" );
	}

	const Money paid = inserted_coins.total();
	if ( paid < ticket->price ) {
		return fail( PurchaseError::InsufficientFunds, "Paid amount less than ticket price" );
	}

	const Money change_amount = paid - ticket->price;

	CoinInventory temp_cashbox = cashbox_;
	temp_cashbox.addCoins( inserted_coins );

	auto change = ChangeCalculator::computeMinimalCoinChange( change_amount, temp_cashbox );
	if ( !change ) {
		return fail( PurchaseError::CannotMakeChange, "No change available for this amount" );
	}

	cashbox_.addCoins( inserted_coins );
	cashbox_.removeCoins( change->coins );

	ticket->status = TicketStatus::Sold;
	ticket->owner  = customer;

	removeReservation( reservation_id );

	return PurchaseSuccess{ .ticket_id   = ticket->id,
	                        .paid        = paid,
	                        .price       = ticket->price,
	                        .ticket_type = ticket->type,
	                        .customer    = customer,
	                        .change      = std::move( *change ) };
}

void TicketServer::cleanupExpiredReservations() {
	const auto now = clock_();

	std::erase_if( reservations_, [ & ]( const Reservation& res ) {
		if ( res.expires_at <= now ) {
			if ( Ticket* ticket = findTicketById( res.ticket_id );
			     ticket && ticket->status == TicketStatus::Reserved ) {
				ticket->status = TicketStatus::Available;
			}
			expired_reservation_ids_.insert( res.id );
			return true;
		}
		return false;
	} );
}

void TicketServer::removeReservation( ReservationId reservation_id ) {
	reservations_.erase( std::remove_if( reservations_.begin(), reservations_.end(),
	                                     [ reservation_id ]( const auto& r ) { return r.id == reservation_id; } ),
	                     reservations_.end() );
}

std::expected< Ticket*, std::monostate > TicketServer::findAvailableTicketByType( const std::string& ticket_type ) {
	for ( auto& ticket : tickets_ ) {
		if ( ticket.type == ticket_type && ticket.status == TicketStatus::Available ) {
			return &ticket;
		}
	}
	return std::unexpected( std::monostate{} );
}

Ticket* TicketServer::findTicketById( TicketId id ) {
	for ( auto& ticket : tickets_ ) {
		if ( ticket.id == id ) {
			return &ticket;
		}
	}
	return nullptr;
}

Reservation* TicketServer::findReservationById( ReservationId id ) {
	for ( auto& reservation : reservations_ ) {
		if ( reservation.id == id ) {
			return &reservation;
		}
	}
	return nullptr;
}

bool TicketServer::isReservationExpired( ReservationId reservation_id ) const {
	return expired_reservation_ids_.contains( reservation_id );
}

}  // namespace task1
