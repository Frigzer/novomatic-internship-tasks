#include "network_protocol.hpp"

#include <stdexcept>

namespace task1::protocol {
namespace {

Json requireObject( const Json& json, const std::string& field ) {
	if ( !json.contains( field ) ) {
		throw std::runtime_error( "Missing field: " + field );
	}
	return json.at( field );
}

}  // namespace

Json makeErrorResponse( const std::string& message ) {
	return Json{ { "ok", false }, { "message", message } };
}

Json makeOkResponse() {
	return Json{ { "ok", true } };
}

Json toJson( const CustomerData& customer ) {
	return Json{ { "first_name", customer.first_name }, { "last_name", customer.last_name } };
}

CustomerData customerFromJson( const Json& json ) {
	return CustomerData{ .first_name = json.at( "first_name" ).get< std::string >(),
	                     .last_name  = json.at( "last_name" ).get< std::string >() };
}

Json toJson( const std::map< Money, int, std::greater<> >& coins ) {
	Json result = Json::array();
	for ( const auto& [ denomination, count ] : coins ) {
		result.push_back( Json{ { "denomination", denomination }, { "count", count } } );
	}
	return result;
}

std::map< Money, int, std::greater<> > coinMapFromJson( const Json& json ) {
	if ( !json.is_array() ) {
		throw std::runtime_error( "Coin list must be an array" );
	}

	std::map< Money, int, std::greater<> > result;
	for ( const auto& entry : json ) {
		const auto denomination = entry.at( "denomination" ).get< Money >();
		const auto count        = entry.at( "count" ).get< int >();
		result[ denomination ] += count;
	}
	return result;
}

Json toJson( const CoinInventory& inventory ) {
	return toJson( inventory.getCoins() );
}

CoinInventory coinInventoryFromJson( const Json& json ) {
	return CoinInventory( coinMapFromJson( json ) );
}

Json toJson( const ChangeResult& change ) {
	return Json{ { "total", change.total }, { "coins", toJson( change.coins ) } };
}

Json toJson( const ReservationResult& reservation ) {
	return Json{ { "ticket_type", reservation.ticket_type },
	             { "reservation_id", reservation.reservation_id },
	             { "ticket_id", reservation.ticket_id },
	             { "price", reservation.price } };
}

Json toJson( const TicketAvailability& availability ) {
	return Json{ { "type", availability.type },
	             { "price", availability.price },
	             { "available_count", availability.available_count } };
}

Json toJson( const PurchaseSuccess& success ) {
	return Json{ { "ticket_id", success.ticket_id },
	             { "paid", success.paid },
	             { "price", success.price },
	             { "ticket_type", success.ticket_type },
	             { "customer", toJson( success.customer ) },
	             { "change", toJson( success.change ) } };
}

Json toJson( const PurchaseFailure& failure ) {
	return Json{ { "error", toString( failure.error ) },
	             { "returned_coins", toJson( failure.returned_coins ) },
	             { "message", failure.message } };
}

PurchaseSuccess purchaseSuccessFromJson( const Json& json ) {
	const auto& change = json.at( "change" );
	return PurchaseSuccess{ .ticket_id   = json.at( "ticket_id" ).get< TicketId >(),
	                        .paid        = json.at( "paid" ).get< Money >(),
	                        .price       = json.at( "price" ).get< Money >(),
	                        .ticket_type = json.at( "ticket_type" ).get< std::string >(),
	                        .customer    = customerFromJson( json.at( "customer" ) ),
	                        .change      = ChangeResult{ .total = change.at( "total" ).get< Money >(),
	                                                     .coins = coinMapFromJson( change.at( "coins" ) ) } };
}

PurchaseFailure purchaseFailureFromJson( const Json& json ) {
	return PurchaseFailure{ .error          = purchaseErrorFromString( json.at( "error" ).get< std::string >() ),
	                        .returned_coins = coinMapFromJson( json.at( "returned_coins" ) ),
	                        .message        = json.at( "message" ).get< std::string >() };
}

ReservationResult reservationFromJson( const Json& json ) {
	return ReservationResult{ .ticket_type    = json.at( "ticket_type" ).get< std::string >(),
	                          .reservation_id = json.at( "reservation_id" ).get< ReservationId >(),
	                          .ticket_id      = json.at( "ticket_id" ).get< TicketId >(),
	                          .price          = json.at( "price" ).get< Money >() };
}

std::vector< TicketAvailability > availabilityListFromJson( const Json& json ) {
	if ( !json.is_array() ) {
		throw std::runtime_error( "Availability payload must be an array" );
	}

	std::vector< TicketAvailability > result;
	result.reserve( json.size() );

	for ( const auto& entry : json ) {
		result.push_back( TicketAvailability{ .type            = entry.at( "type" ).get< std::string >(),
		                                      .price           = entry.at( "price" ).get< Money >(),
		                                      .available_count = entry.at( "available_count" ).get< int >() } );
	}

	return result;
}

std::string toString( PurchaseError error ) {
	switch ( error ) {
	case PurchaseError::ReservationNotFound: return "reservation_not_found";
	case PurchaseError::ReservationExpired: return "reservation_expired";
	case PurchaseError::InsufficientFunds: return "insufficient_funds";
	case PurchaseError::CannotMakeChange: return "cannot_make_change";
	}

	throw std::runtime_error( "Unknown purchase error enum value" );
}

PurchaseError purchaseErrorFromString( const std::string& value ) {
	if ( value == "reservation_not_found" ) return PurchaseError::ReservationNotFound;
	if ( value == "reservation_expired" ) return PurchaseError::ReservationExpired;
	if ( value == "insufficient_funds" ) return PurchaseError::InsufficientFunds;
	if ( value == "cannot_make_change" ) return PurchaseError::CannotMakeChange;

	throw std::runtime_error( "Unknown purchase error string: " + value );
}

}  // namespace task1::protocol
