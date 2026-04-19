#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace task1 {

class TicketMachineClient;

class ConsoleClientApp {
public:
	ConsoleClientApp( int argc, char* argv[] );

	int run();

private:
	struct SessionState {
		std::optional< ReservationResult > reservation;
	};

	void parseArguments();
	[[nodiscard]] bool shouldPrintHelp() const noexcept;
	void printHelp() const;
	void printWelcome() const;
	void printCommandHelp() const;
	void commandLoop( TicketMachineClient& client );
	void handleCommand( TicketMachineClient& client, const std::string& line, bool& should_exit );
	void handleReserve( TicketMachineClient& client, const std::string& ticket_type );
	void handleBuy( TicketMachineClient& client );
	void handleCancel( TicketMachineClient& client );
	void printStatus() const;
	static void printAvailability( TicketMachineClient& client );
	static void printPurchaseSuccess( const PurchaseSuccess& success );
	static void printPurchaseFailure( const PurchaseFailure& failure );
	static void printMoney( Money amount );
	static std::optional< CoinInventory > readInsertedCoins();
	static std::optional< CustomerData > readCustomerData();

	int argc_{ 0 };
	char** argv_{ nullptr };
	std::string host_{ "127.0.0.1" };
	std::uint16_t port_{ 5555 };
	bool show_help_{ false };
	SessionState session_{};
};

}  // namespace task1
