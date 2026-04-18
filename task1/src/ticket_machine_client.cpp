#include "ticket_machine_client.hpp"

#include "network_protocol.hpp"
#include "ticket_server.hpp"

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include <istream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace task1 {
namespace {
using asio::ip::tcp;
using Json = nlohmann::json;

class RemoteClientImpl {
public:
	RemoteClientImpl( std::string host, std::uint16_t port )
	    : host_( std::move( host ) ), port_( port ), resolver_( io_context_ ), socket_( io_context_ ) {
		connect();
	}

	~RemoteClientImpl() {
		asio::error_code ignored;
		socket_.close( ignored );
	}

	RemoteClientImpl( const RemoteClientImpl& )            = delete;
	RemoteClientImpl& operator=( const RemoteClientImpl& ) = delete;

	Json request( const Json& payload ) {
		const auto serialized = payload.dump() + "\n";
		asio::error_code error;
		asio::write( socket_, asio::buffer( serialized ), error );
		if ( error ) {
			throw std::runtime_error( "Could not send request: " + error.message() );
		}

		asio::read_until( socket_, input_buffer_, '\n', error );
		if ( error ) {
			throw std::runtime_error( "Could not read response: " + error.message() );
		}

		std::istream stream( &input_buffer_ );
		std::string line;
		std::getline( stream, line );
		if ( line.empty() ) {
			throw std::runtime_error( "Server returned an empty response" );
		}

		Json response = Json::parse( line );
		if ( !response.value( "ok", false ) ) {
			throw std::runtime_error( response.value( "message", std::string( "Server returned an error" ) ) );
		}
		return response;
	}

private:
	void connect() {
		asio::error_code error;
		auto endpoints = resolver_.resolve( host_, std::to_string( port_ ), error );
		if ( error ) {
			throw std::runtime_error( "Could not resolve server address: " + error.message() );
		}

		asio::connect( socket_, endpoints, error );
		if ( error ) {
			throw std::runtime_error( "Could not connect to server: " + error.message() );
		}
	}

	std::string host_;
	std::uint16_t port_{ 0 };
	asio::io_context io_context_;
	tcp::resolver resolver_;
	tcp::socket socket_;
	asio::streambuf input_buffer_;
};

}  // namespace

class TicketMachineClient::Impl {
public:
	explicit Impl( std::string host, std::uint16_t port ) : remote_( std::move( host ), port ) {}

	std::vector< TicketAvailability > showAvailableTickets() {
		const auto response = remote_.request( Json{ { "action", "list_tickets" } } );
		return protocol::availabilityListFromJson( response.at( "tickets" ) );
	}

	std::optional< ReservationResult > selectTicket( std::string_view ticket_type ) {
		const auto response = remote_.request( Json{ { "action", "reserve_ticket" }, { "ticket_type", ticket_type } } );
		if ( !response.at( "reserved" ).get< bool >() ) {
			return std::nullopt;
		}
		return protocol::reservationFromJson( response.at( "reservation" ) );
	}

	bool cancel( ReservationId reservation_id ) {
		const auto response = remote_.request(
		    Json{ { "action", "cancel_reservation" }, { "reservation_id", reservation_id } } );
		return response.at( "cancelled" ).get< bool >();
	}

	std::variant< PurchaseSuccess, PurchaseFailure > buy( ReservationId reservation_id, const CustomerData& customer,
	                                                     const CoinInventory& inserted_coins ) {
		Json response = remote_.request( Json{ { "action", "finalize_purchase" },
		                                     { "reservation_id", reservation_id },
		                                     { "customer", protocol::toJson( customer ) },
		                                     { "inserted_coins", protocol::toJson( inserted_coins ) } } );
		if ( response.at( "success" ).get< bool >() ) {
			return protocol::purchaseSuccessFromJson( response.at( "purchase" ) );
		}
		return protocol::purchaseFailureFromJson( response.at( "purchase" ) );
	}

private:
	RemoteClientImpl remote_;
};

TicketMachineClient::TicketMachineClient( TicketServer& server ) : local_server_( &server ) {}

TicketMachineClient::TicketMachineClient( std::string host, std::uint16_t port )
    : impl_( std::make_unique< Impl >( std::move( host ), port ) ) {}

TicketMachineClient::~TicketMachineClient() = default;

TicketMachineClient::TicketMachineClient( TicketMachineClient&& ) noexcept            = default;
TicketMachineClient& TicketMachineClient::operator=( TicketMachineClient&& ) noexcept = default;

std::vector< TicketAvailability > TicketMachineClient::showAvailableTickets() {
	if ( local_server_ != nullptr ) {
		return local_server_->getAvailableTickets();
	}
	return impl_->showAvailableTickets();
}

std::optional< ReservationResult > TicketMachineClient::selectTicket( std::string_view ticket_type ) {
	if ( local_server_ != nullptr ) {
		return local_server_->reserveTicket( std::string( ticket_type ) );
	}
	return impl_->selectTicket( ticket_type );
}

bool TicketMachineClient::cancel( ReservationId reservation_id ) {
	if ( local_server_ != nullptr ) {
		return local_server_->cancelReservation( reservation_id );
	}
	return impl_->cancel( reservation_id );
}

std::variant< PurchaseSuccess, PurchaseFailure > TicketMachineClient::buy( ReservationId reservation_id,
                                                                           const CustomerData& customer,
                                                                           const CoinInventory& inserted_coins ) {
	if ( local_server_ != nullptr ) {
		return local_server_->finalizePurchase( reservation_id, customer, inserted_coins );
	}
	return impl_->buy( reservation_id, customer, inserted_coins );
}

}  // namespace task1
