#include "server_seed_data.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace task1 {
namespace {

using json = nlohmann::json;

TicketStatus parseTicketStatus( const std::string& value ) {
	if ( value == "available" ) {
		return TicketStatus::Available;
	}
	if ( value == "reserved" ) {
		return TicketStatus::Reserved;
	}
	if ( value == "sold" ) {
		return TicketStatus::Sold;
	}

	throw std::runtime_error( "Invalid ticket status in seed data: " + value );
}

CustomerData parseCustomerData( const json& value ) {
	if ( !value.is_object() ) {
		throw std::runtime_error( "Ticket owner must be a JSON object" );
	}

	return CustomerData{
	    .first_name = value.at( "first_name" ).get< std::string >(),
	    .last_name  = value.at( "last_name" ).get< std::string >(),
	};
}

Ticket parseTicket( const json& value ) {
	if ( !value.is_object() ) {
		throw std::runtime_error( "Each ticket entry must be a JSON object" );
	}

	Ticket ticket{
	    .id     = value.at( "id" ).get< TicketId >(),
	    .price  = value.at( "price" ).get< Money >(),
	    .type   = value.at( "type" ).get< std::string >(),
	    .status = parseTicketStatus( value.value( "status", std::string{ "available" } ) ),
	    .owner  = std::nullopt,
	};

	if ( ticket.id == 0 ) {
		throw std::runtime_error( "Ticket id must be positive" );
	}
	if ( ticket.price <= 0 ) {
		throw std::runtime_error( "Ticket price must be positive" );
	}
	if ( ticket.type.empty() ) {
		throw std::runtime_error( "Ticket type cannot be empty" );
	}

	if ( value.contains( "owner" ) && !value.at( "owner" ).is_null() ) {
		ticket.owner = parseCustomerData( value.at( "owner" ) );
	}

	return ticket;
}

CoinInventory parseCashbox( const json& value ) {
	if ( !value.is_array() ) {
		throw std::runtime_error( "cashbox must be a JSON array" );
	}

	std::map< Money, int, std::greater<> > coins;
	for ( const auto& entry : value ) {
		if ( !entry.is_object() ) {
			throw std::runtime_error( "Each cashbox entry must be a JSON object" );
		}

		const auto denomination = entry.at( "denomination" ).get< Money >();
		const auto count        = entry.at( "count" ).get< int >();

		if ( denomination <= 0 ) {
			throw std::runtime_error( "Cashbox denomination must be positive" );
		}
		if ( count < 0 ) {
			throw std::runtime_error( "Cashbox coin count cannot be negative" );
		}

		coins[ denomination ] += count;
	}

	return CoinInventory( std::move( coins ) );
}

void validateTickets( const std::vector< Ticket >& tickets ) {
	std::unordered_set< TicketId > ids;
	for ( const auto& ticket : tickets ) {
		if ( !ids.insert( ticket.id ).second ) {
			throw std::runtime_error( "Duplicate ticket id in seed data: " + std::to_string( ticket.id ) );
		}
	}
}

}  // namespace

ServerSeedData ServerSeedDataLoader::loadFromFile( const std::filesystem::path& file_path ) {
	std::ifstream input( file_path );
	if ( !input ) {
		throw std::runtime_error( "Could not open server seed data file: " + file_path.string() );
	}

	json root;
	input >> root;

	if ( !root.is_object() ) {
		throw std::runtime_error( "Server seed data root must be a JSON object" );
	}

	if ( !root.contains( "tickets" ) ) {
		throw std::runtime_error( "Server seed data must contain 'tickets'" );
	}
	if ( !root.contains( "cashbox" ) ) {
		throw std::runtime_error( "Server seed data must contain 'cashbox'" );
	}

	std::vector< Ticket > tickets;
	for ( const auto& ticket_json : root.at( "tickets" ) ) {
		tickets.push_back( parseTicket( ticket_json ) );
	}
	validateTickets( tickets );

	return ServerSeedData{
	    .tickets = std::move( tickets ),
	    .cashbox = parseCashbox( root.at( "cashbox" ) ),
	};
}

}  // namespace task1
