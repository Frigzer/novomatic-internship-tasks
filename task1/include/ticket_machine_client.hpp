#pragma once

#include "ticket_server.hpp"

#include <asio.hpp>
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace task1 {

class TicketMachineClient {
public:
	explicit TicketMachineClient( TicketServer& server );
	TicketMachineClient( std::string host, std::uint16_t port );
	~TicketMachineClient();

	TicketMachineClient( const TicketMachineClient& )            = delete;
	TicketMachineClient& operator=( const TicketMachineClient& ) = delete;
	TicketMachineClient( TicketMachineClient&& ) noexcept;
	TicketMachineClient& operator=( TicketMachineClient&& ) noexcept;

	std::vector< TicketAvailability > showAvailableTickets();
	std::optional< ReservationResult > selectTicket( std::string_view ticket_type );
	bool cancel( ReservationId reservation_id );
	std::variant< PurchaseSuccess, PurchaseFailure > buy( ReservationId reservation_id, const CustomerData& customer,
	                                                      const CoinInventory& inserted_coins );
	void ping();

private:
	struct RemoteEndpoint {
		std::string host;
		std::uint16_t port{ 0 };
	};

	struct RemoteConnection {
		asio::io_context io_context;
		asio::ip::tcp::resolver resolver{ io_context };
		asio::ip::tcp::socket socket{ io_context };
		bool connected{ false };
	};

	[[nodiscard]] bool isRemote() const noexcept;
	[[nodiscard]] TicketServer& localServer() const;
	[[nodiscard]] const RemoteEndpoint& remoteEndpoint() const;
	[[nodiscard]] RemoteConnection& remoteConnection();
	nlohmann::json sendRemoteRequest( const nlohmann::json& request );
	void ensureConnected();
	void closeRemoteConnection() noexcept;

	std::variant< TicketServer*, RemoteEndpoint > backend_;
	std::unique_ptr< RemoteConnection > remote_connection_;
};

}  // namespace task1
