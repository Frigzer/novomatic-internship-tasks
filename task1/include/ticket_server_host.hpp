#pragma once

#include <cstdint>
#include <memory>

namespace task1 {

class TicketServer;

class TicketServerHost {
public:
	TicketServerHost( TicketServer& server, std::uint16_t port );
	~TicketServerHost();

	TicketServerHost( const TicketServerHost& )            = delete;
	TicketServerHost& operator=( const TicketServerHost& ) = delete;
	TicketServerHost( TicketServerHost&& )                 = delete;
	TicketServerHost& operator=( TicketServerHost&& )      = delete;

	void start();
	void stop();

	[[nodiscard]] std::uint16_t port() const;
	[[nodiscard]] bool isRunning() const;

private:
	class Impl;
	std::unique_ptr< Impl > impl_;
};

}  // namespace task1
