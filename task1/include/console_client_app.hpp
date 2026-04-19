#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace task1 {

class TicketMachineClient;

class ConsoleClientApp {
public:
	explicit ConsoleClientApp( std::span< char* const > args );

	int run();

private:
	struct SessionState {
		std::optional< ReservationResult > reservation;
	};

	static constexpr std::uint16_t defaultPort = 5555;

	void parseArguments();
	[[nodiscard]] bool shouldPrintHelp() const noexcept;
	void printWelcome() const;
	void commandLoop( TicketMachineClient& client );
	void handleCommand( TicketMachineClient& client, const std::string& line, bool& should_exit );
	void handleReserve( TicketMachineClient& client, const std::string& ticket_type );
	void handleBuy( TicketMachineClient& client );
	void handleCancel( TicketMachineClient& client );
	void printStatus() const;
	static void printAvailability( TicketMachineClient& client );
	static void printPurchaseSuccess( const PurchaseSuccess& success );
	static void printPurchaseFailure( const PurchaseFailure& failure );
	static std::optional< CoinInventory > readInsertedCoins();
	static std::optional< CustomerData > readCustomerData();

	std::span< char* const > args_;
	std::string host_{ "127.0.0.1" };
	std::uint16_t port_{ defaultPort };
	bool show_help_{ false };
	SessionState session_{};
};

}  // namespace task1