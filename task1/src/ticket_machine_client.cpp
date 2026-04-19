#include "ticket_machine_client.hpp"

#include "network_protocol.hpp"

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include <istream>
#include <stdexcept>
#include <string>
#include <utility>

namespace task1 {
namespace {
using asio::ip::tcp;
using Json = nlohmann::json;

void ensureOkResponse( const Json& response ) {
	if ( !response.value( "ok", false ) ) {
		throw std::runtime_error( response.value( "message", std::string( "Unknown server error" ) ) );
	}
}

}  // namespace

TicketMachineClient::TicketMachineClient( TicketServer& server ) : backend_( &server ) {}

TicketMachineClient::TicketMachineClient( std::string host, std::uint16_t port )
    : backend_( RemoteEndpoint{ .host = std::move( host ), .port = port } ),
      remote_connection_( std::make_unique< RemoteConnection >() ) {}

TicketMachineClient::~TicketMachineClient() {
	closeRemoteConnection();
}

TicketMachineClient::TicketMachineClient( TicketMachineClient&& other ) noexcept
    : backend_( std::move( other.backend_ ) ), remote_connection_( std::move( other.remote_connection_ ) ) {}

TicketMachineClient& TicketMachineClient::operator=( TicketMachineClient&& other ) noexcept {
	if ( this == &other ) {
		return *this;
	}

	closeRemoteConnection();
	backend_           = std::move( other.backend_ );
	remote_connection_ = std::move( other.remote_connection_ );
	return *this;
}

std::vector< TicketAvailability > TicketMachineClient::showAvailableTickets() {
	if ( !isRemote() ) {
		return localServer().getAvailableTickets();
	}

	const Json response = sendRemoteRequest( Json{ { "action", "list_tickets" } } );
	ensureOkResponse( response );
	return protocol::availabilityListFromJson( response.at( "tickets" ) );
}

std::optional< ReservationResult > TicketMachineClient::selectTicket( std::string_view ticket_type ) {
	if ( !isRemote() ) {
		return localServer().reserveTicket( std::string( ticket_type ) );
	}

	const Json response =
	    sendRemoteRequest( Json{ { "action", "reserve_ticket" }, { "ticket_type", std::string( ticket_type ) } } );
	ensureOkResponse( response );
	if ( !response.value( "reserved", false ) ) {
		return std::nullopt;
	}
	return protocol::reservationFromJson( response.at( "reservation" ) );
}

bool TicketMachineClient::cancel( ReservationId reservation_id ) {
	if ( !isRemote() ) {
		return localServer().cancelReservation( reservation_id );
	}

	const Json response =
	    sendRemoteRequest( Json{ { "action", "cancel_reservation" }, { "reservation_id", reservation_id } } );
	ensureOkResponse( response );
	return response.at( "cancelled" ).get< bool >();
}

std::variant< PurchaseSuccess, PurchaseFailure > TicketMachineClient::buy( ReservationId reservation_id,
                                                                           const CustomerData& customer,
                                                                           const CoinInventory& inserted_coins ) {
	if ( !isRemote() ) {
		return localServer().finalizePurchase( reservation_id, customer, inserted_coins );
	}

	const Json response = sendRemoteRequest( Json{ { "action", "finalize_purchase" },
	                                               { "reservation_id", reservation_id },
	                                               { "customer", protocol::toJson( customer ) },
	                                               { "inserted_coins", protocol::toJson( inserted_coins ) } } );
	ensureOkResponse( response );

	if ( response.at( "success" ).get< bool >() ) {
		return protocol::purchaseSuccessFromJson( response.at( "purchase" ) );
	}
	return protocol::purchaseFailureFromJson( response.at( "purchase" ) );
}

bool TicketMachineClient::isRemote() const noexcept {
	return std::holds_alternative< RemoteEndpoint >( backend_ );
}

TicketServer& TicketMachineClient::localServer() const {
	auto* server = std::get_if< TicketServer* >( &backend_ );
	if ( server == nullptr || *server == nullptr ) {
		throw std::logic_error( "Local ticket server backend is not available" );
	}
	return **server;
}

const TicketMachineClient::RemoteEndpoint& TicketMachineClient::remoteEndpoint() const {
	const auto* endpoint = std::get_if< RemoteEndpoint >( &backend_ );
	if ( endpoint == nullptr ) {
		throw std::logic_error( "Remote ticket server backend is not available" );
	}
	return *endpoint;
}

TicketMachineClient::RemoteConnection& TicketMachineClient::remoteConnection() {
	if ( !isRemote() || remote_connection_ == nullptr ) {
		throw std::logic_error( "Remote ticket server connection is not available" );
	}
	return *remote_connection_;
}

void TicketMachineClient::ping() {
	if ( !isRemote() ) {
		return;
	}

	const auto response = sendRemoteRequest( Json{ { "action", "ping" } } );
	const bool ok       = response.at( "ok" ).get< bool >();
	if ( !ok ) {
		throw std::runtime_error( response.value( "message", std::string{ "Ping request failed" } ) );
	}
}

void TicketMachineClient::ensureConnected() {
	auto& connection = remoteConnection();
	if ( connection.connected && connection.socket.is_open() ) {
		return;
	}

	const auto& endpoint = remoteEndpoint();
	asio::error_code error;
	const auto endpoints = connection.resolver.resolve( endpoint.host, std::to_string( endpoint.port ), error );
	if ( error ) {
		throw std::runtime_error( "Could not resolve server address: " + error.message() );
	}

	asio::connect( connection.socket, endpoints, error );
	if ( error ) {
		throw std::runtime_error( "Could not connect to server: " + error.message() );
	}

	connection.connected = true;
}

void TicketMachineClient::closeRemoteConnection() noexcept {
	if ( remote_connection_ == nullptr ) {
		return;
	}

	asio::error_code ignored;
	remote_connection_->socket.shutdown( tcp::socket::shutdown_both, ignored );
	remote_connection_->socket.close( ignored );
	remote_connection_->connected = false;
}

Json TicketMachineClient::sendRemoteRequest( const Json& request ) {
	ensureConnected();

	auto& connection = remoteConnection();
	asio::error_code error;

	const auto payload = request.dump() + "\n";
	asio::write( connection.socket, asio::buffer( payload ), error );
	if ( error ) {
		closeRemoteConnection();
		throw std::runtime_error( "Could not send request to server: " + error.message() );
	}

	asio::streambuf response_buffer;
	asio::read_until( connection.socket, response_buffer, '\n', error );
	if ( error ) {
		closeRemoteConnection();
		throw std::runtime_error( "Could not read response from server: " + error.message() );
	}

	std::istream response_stream( &response_buffer );
	std::string response_line;
	std::getline( response_stream, response_line );
	if ( response_line.empty() ) {
		throw std::runtime_error( "Received an empty response from the server" );
	}

	return Json::parse( response_line );
}

}  // namespace task1
