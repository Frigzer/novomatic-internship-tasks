#include "ticket_server_host.hpp"

#include "network_protocol.hpp"
#include "ticket_server.hpp"

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

namespace task1 {
namespace {
using asio::ip::tcp;
using Json = nlohmann::json;

class Session : public std::enable_shared_from_this< Session > {
public:
	Session( tcp::socket socket, TicketServer& server ) : socket_( std::move( socket ) ), server_( server ) {}

	void start() {
		readNextRequest();
	}

	void stop() {
		asio::error_code ignored;
		socket_.close( ignored );
	}

private:
	void readNextRequest() {
		auto self = shared_from_this();
		asio::async_read_until( socket_, input_buffer_, '\n',
		                       [ self ]( const asio::error_code& error, std::size_t ) {
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

		if ( action == "list_tickets" ) {
			Json response = protocol::makeOkResponse();
			response[ "tickets" ] = Json::array();
			for ( const auto& item : server_.getAvailableTickets() ) {
				response[ "tickets" ].push_back( protocol::toJson( item ) );
			}
			return response;
		}

		if ( action == "reserve_ticket" ) {
			const auto ticket_type = request.at( "ticket_type" ).get< std::string >();
			auto reservation       = server_.reserveTicket( ticket_type );
			if ( !reservation.has_value() ) {
				return Json{ { "ok", true }, { "reserved", false } };
			}
			return Json{ { "ok", true }, { "reserved", true }, { "reservation", protocol::toJson( *reservation ) } };
		}

		if ( action == "cancel_reservation" ) {
			const auto reservation_id = request.at( "reservation_id" ).get< ReservationId >();
			return Json{ { "ok", true }, { "cancelled", server_.cancelReservation( reservation_id ) } };
		}

		if ( action == "finalize_purchase" ) {
			const auto reservation_id = request.at( "reservation_id" ).get< ReservationId >();
			const auto customer       = protocol::customerFromJson( request.at( "customer" ) );
			const auto inserted       = protocol::coinInventoryFromJson( request.at( "inserted_coins" ) );
			auto result               = server_.finalizePurchase( reservation_id, customer, inserted );

			Json response = protocol::makeOkResponse();
			if ( std::holds_alternative< PurchaseSuccess >( result ) ) {
				response[ "success" ] = true;
				response[ "purchase" ] = protocol::toJson( std::get< PurchaseSuccess >( result ) );
			} else {
				response[ "success" ] = false;
				response[ "purchase" ] = protocol::toJson( std::get< PurchaseFailure >( result ) );
			}
			return response;
		}

		return protocol::makeErrorResponse( "Unknown action: " + action );
	}

	tcp::socket socket_;
	asio::streambuf input_buffer_;
	TicketServer& server_;
};

}  // namespace

class TicketServerHost::Impl {
public:
	Impl( TicketServer& server, std::uint16_t port )
	    : server_( server ), requested_port_( port ), acceptor_( io_context_ ) {}

	~Impl() {
		stop();
	}

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
		scheduleAccept();
		worker_ = std::thread( [ this ]() { io_context_.run(); } );
	}

	void stop() {
		if ( !running_.exchange( false ) ) {
			return;
		}

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

	[[nodiscard]] std::uint16_t port() const {
		return bound_port_;
	}

	[[nodiscard]] bool isRunning() const {
		return running_.load();
	}

private:
	void scheduleAccept() {
		acceptor_.async_accept( [ this ]( const asio::error_code& error, tcp::socket socket ) {
			if ( !error ) {
				auto session = std::make_shared< Session >( std::move( socket ), server_ );
				{
					std::scoped_lock lock( sessions_mutex_ );
					sessions_.push_back( session );
				}
				session->start();
			}

			cleanupExpiredSessions();
			if ( running_.load() && acceptor_.is_open() ) {
				scheduleAccept();
			}
		} );
	}

	void cleanupExpiredSessions() {
		std::scoped_lock lock( sessions_mutex_ );
		std::erase_if( sessions_, []( const std::weak_ptr< Session >& session ) { return session.expired(); } );
	}

	TicketServer& server_;
	std::uint16_t requested_port_{ 0 };
	std::uint16_t bound_port_{ 0 };
	asio::io_context io_context_;
	tcp::acceptor acceptor_;
	std::optional< asio::executor_work_guard< asio::io_context::executor_type > > work_guard_;
	std::thread worker_;
	std::atomic_bool running_{ false };
	std::mutex sessions_mutex_;
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
