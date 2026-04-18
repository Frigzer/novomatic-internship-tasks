#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <nlohmann/json.hpp>

namespace task1::protocol {

using Json = nlohmann::json;

Json makeErrorResponse( const std::string& message );
Json makeOkResponse();

Json toJson( const CustomerData& customer );
CustomerData customerFromJson( const Json& json );

Json toJson( const std::map< Money, int, std::greater<> >& coins );
std::map< Money, int, std::greater<> > coinMapFromJson( const Json& json );

Json toJson( const CoinInventory& inventory );
CoinInventory coinInventoryFromJson( const Json& json );

Json toJson( const ChangeResult& change );
Json toJson( const ReservationResult& reservation );
Json toJson( const TicketAvailability& availability );
Json toJson( const PurchaseSuccess& success );
Json toJson( const PurchaseFailure& failure );

PurchaseSuccess purchaseSuccessFromJson( const Json& json );
PurchaseFailure purchaseFailureFromJson( const Json& json );
ReservationResult reservationFromJson( const Json& json );
std::vector< TicketAvailability > availabilityListFromJson( const Json& json );

std::string toString( PurchaseError error );
PurchaseError purchaseErrorFromString( const std::string& value );

}  // namespace task1::protocol
