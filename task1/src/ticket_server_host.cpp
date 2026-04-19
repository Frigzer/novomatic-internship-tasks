#include "ticket_server_host.hpp"

#include "network_protocol.hpp"
#include "ticket_server.hpp"

#include <asio.hpp>

#include <algorithm>
#include <nlohmann/json.hpp>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace task1 {
namespace {
using asio::ip::tcp;
using Json = nlohmann::json;

std::mutex g_log_mutex;

void logMessage( const std::string& message ) {
	std::lock_guard< std::mutex > lock( g_log_mutex );
	std::cout << "[server] " << message << '\n';
}

std::string endpointToString( const tcp::socket& socket ) {
	asio::error_code error;
	const auto endpoint = socket.remote_endpoint( error );
	if ( error ) {
		return "unknown-client";
	}

	std::ostringstream stream;
	stream << endpoint.address().to_string() << ':' << endpoint.port();
	return stream.str();
}

class Session : public std::enable_shared_from_this< Session > {
public:
	Session( tcp::socket socket, TicketServer& server )
	    : socket_( std::move( socket ) ), server_( server ), client_label_( endpointToString( socket_ ) ) {}

	void start() {
		logMessage( "Client connected: " + client_label_ );
		readNextRequest();
	}

	void stop() {
		if ( stopped_ ) {
			return;
		}

		stopped_ = true;
		asio::error_code ignored;
		socket_.close( ignored );
		logMessage( "Client disconnected: " + client_label_ );
	}

private:
	void readNextRequest() {
		auto self = shared_from_this();
		asio::async_read_until( socket_, input_buffer_, '\n', [ self ]( const asio::error_code& error, std::size_t ) {
			if ( error ) {
				self->stop();
				return;
			}

			std::istream stream( &self->input_buffer_ );
			std::string line;
			std::getline( stream, line );

			if ( line.empty() ) {
				self->readNextRequest();
				return;
			}

			Json response;
			try {
				response = self->handleRequest( Json::parse( line ) );
			} catch ( const std::exception& exception ) {
				response = protocol::makeErrorResponse( exception.what() );
			}

			self->writeResponse( response.dump() + "\n" );
		} );
	}

	void writeResponse( std::string payload ) {
		auto self   = shared_from_this();
		auto buffer = std::make_shared< std::string >( std::move( payload ) );
		asio::async_write( socket_, asio::buffer( *buffer ),
		                   [ self, buffer ]( const asio::error_code& error, std::size_t ) {
			                   if ( error ) {
				                   self->stop();
				                   return;
			                   }
			                   self->readNextRequest();
		                   } );
	}

	Json handleRequest( const Json& request ) {
		const auto action = request.at( "action" ).get< std::string >();
		logMessage( "Request from " + client_label_ + ": " + action );

		if ( action == "ping" ) {
			return Json{ { "ok", true }, { "message", "pong" } };
		}

		if ( action == "list_tickets" ) {
			const auto availability = server_.getAvailableTickets();
			logMessage( "Listing available tickets for " + client_label_ + ": " +
			            std::to_string( availability.size() ) + " ticket groups visible" );

			Json response         = protocol::makeOkResponse();
			response[ "tickets" ] = Json::array();
			for ( const auto& item : availability ) {
				response[ "tickets" ].push_back( protocol::toJson( item ) );
			}
			return response;
		}

		if ( action == "reserve_ticket" ) {
			const auto ticket_type = request.at( "ticket_type" ).get< std::string >();
			auto reservation       = server_.reserveTicket( ticket_type );
			if ( !reservation.has_value() ) {
				logMessage( "Reservation rejected for " + client_label_ + ": ticket type='" + ticket_type +
				            "' is unavailable" );
				return Json{ { "ok", true }, { "reserved", false } };
			}

			logMessage( "Reservation created for " + client_label_ +
			            ": reservation_id=" + std::to_string( reservation->reservation_id ) + ", ticket_id=" +
			            std::to_string( reservation->ticket_id ) + ", type='" + reservation->ticket_type + "'" );
			return Json{ { "ok", true }, { "reserved", true }, { "reservation", protocol::toJson( *reservation ) } };
		}

		if ( action == "cancel_reservation" ) {
			const auto reservation_id = request.at( "reservation_id" ).get< ReservationId >();
			const bool cancelled      = server_.cancelReservation( reservation_id );
			logMessage( std::string{ "Cancellation for " } + client_label_ + ": reservation_id=" +
			            std::to_string( reservation_id ) + ( cancelled ? " succeeded" : " had no effect" ) );
			return Json{ { "ok", true }, { "cancelled", cancelled } };
		}

		if ( action == "finalize_purchase" ) {
			const auto reservation_id = request.at( "reservation_id" ).get< ReservationId >();
			const auto customer       = protocol::customerFromJson( request.at( "customer" ) );
			const auto inserted       = protocol::coinInventoryFromJson( request.at( "inserted_coins" ) );
			auto result               = server_.finalizePurchase( reservation_id, customer, inserted );

			Json response = protocol::makeOkResponse();
			if ( std::holds_alternative< PurchaseSuccess >( result ) ) {
				const auto& success = std::get< PurchaseSuccess >( result );
				logMessage( "Purchase completed for " + client_label_ +
				            ": ticket_id=" + std::to_string( success.ticket_id ) + ", reservation_id=" +
				            std::to_string( reservation_id ) + ", change=" + std::to_string( success.change.total ) );
				response[ "success" ]  = true;
				response[ "purchase" ] = protocol::toJson( success );
			} else {
				const auto& failure = std::get< PurchaseFailure >( result );
				logMessage( "Purchase failed for " + client_label_ + ": reservation_id=" +
				            std::to_string( reservation_id ) + ", error=" + protocol::toString( failure.error ) );
				response[ "success" ]  = false;
				response[ "purchase" ] = protocol::toJson( failure );
			}
			return response;
		}

		return protocol::makeErrorResponse( "Unknown action: " + action );
	}

	tcp::socket socket_;
	asio::streambuf input_buffer_;
	TicketServer& server_;
	std::string client_label_;
	bool stopped_{ false };
};

}  // namespace

class TicketServerHost::Impl {
public:
	Impl( TicketServer& server, std::uint16_t port )
	    : server_( server ), requested_port_( port ), acceptor_( io_context_ ) {}

	~Impl() { stop(); }

	void start() {
		if ( running_.exchange( true ) ) {
			throw std::runtime_error( "TicketServerHost is already running" );
		}

		io_context_.restart();
		work_guard_.emplace( asio::make_work_guard( io_context_ ) );

		asio::error_code error;
		const tcp::endpoint endpoint( tcp::v4(), requested_port_ );
		acceptor_.open( endpoint.protocol(), error );
		if ( error ) {
			running_ = false;
			throw std::runtime_error( "Could not open acceptor: " + error.message() );
		}

		acceptor_.set_option( tcp::acceptor::reuse_address( true ), error );
		if ( error ) {
			running_ = false;
			throw std::runtime_error( "Could not configure acceptor: " + error.message() );
		}

		acceptor_.bind( endpoint, error );
		if ( error ) {
			running_ = false;
			throw std::runtime_error( "Could not bind acceptor: " + error.message() );
		}

		acceptor_.listen( asio::socket_base::max_listen_connections, error );
		if ( error ) {
			running_ = false;
			throw std::runtime_error( "Could not listen on acceptor: " + error.message() );
		}

		bound_port_ = acceptor_.local_endpoint().port();
		logMessage( "Server is ready on port " + std::to_string( bound_port_ ) );
		scheduleAccept();
		worker_ = std::thread( [ this ]() { io_context_.run(); } );
	}

	void stop() {
		if ( !running_.exchange( false ) ) {
			return;
		}

		logMessage( "Stopping server host" );
		asio::error_code ignored;
		acceptor_.cancel( ignored );
		acceptor_.close( ignored );
		work_guard_.reset();
		io_context_.stop();

		if ( worker_.joinable() ) {
			worker_.join();
		}

		io_context_.restart();
	}

	[[nodiscard]] std::uint16_t port() const { return bound_port_; }

	[[nodiscard]] bool isRunning() const { return running_.load(); }

private:
	void scheduleAccept() {
		acceptor_.async_accept( [ this ]( const asio::error_code& error, tcp::socket socket ) {
			if ( !error ) {
				auto session = std::make_shared< Session >( std::move( socket ), server_ );
				{
					std::lock_guard< std::mutex > lock( sessions_mutex_ );
					sessions_.push_back( session );
				}
				session->start();
			} else if ( running_ ) {
				logMessage( "Accept failed: " + error.message() );
			}

			cleanupSessions();
			if ( running_ ) {
				scheduleAccept();
			}
		} );
	}

	void cleanupSessions() {
		std::lock_guard< std::mutex > lock( sessions_mutex_ );
		sessions_.erase( std::remove_if( sessions_.begin(), sessions_.end(),
		                                 []( const auto& weak_session ) { return weak_session.expired(); } ),
		                 sessions_.end() );
	}

	TicketServer& server_;
	std::uint16_t requested_port_{ 0 };
	std::uint16_t bound_port_{ 0 };
	asio::io_context io_context_;
	tcp::acceptor acceptor_;
	std::optional< asio::executor_work_guard< asio::io_context::executor_type > > work_guard_;
	std::thread worker_;
	std::atomic_bool running_{ false };
	mutable std::mutex sessions_mutex_;
	std::vector< std::weak_ptr< Session > > sessions_;
};

TicketServerHost::TicketServerHost( TicketServer& server, std::uint16_t port )
    : impl_( std::make_unique< Impl >( server, port ) ) {}

TicketServerHost::~TicketServerHost() = default;

void TicketServerHost::start() {
	impl_->start();
}

void TicketServerHost::stop() {
	impl_->stop();
}

std::uint16_t TicketServerHost::port() const {
	return impl_->port();
}

bool TicketServerHost::isRunning() const {
	return impl_->isRunning();
}

}  // namespace task1
