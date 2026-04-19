#include "server_console_app.hpp"

#include "paths.hpp"
#include "server_seed_data.hpp"
#include "ticket_server.hpp"
#include "ticket_server_host.hpp"

#include <chrono>
#include <csignal>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace task1 {
namespace {

volatile std::sig_atomic_t& keepRunningFlag() {
	static volatile std::sig_atomic_t value = 1;
	return value;
}

std::filesystem::path defaultDataFile() {
	return ( paths::dataDir / "server_seed.json" ).lexically_normal();
}

std::filesystem::path resolveDataFilePath( const std::filesystem::path& input ) {
	if ( input.empty() ) {
		return defaultDataFile();
	}

	if ( input.is_absolute() ) {
		return input.lexically_normal();
	}

	return ( paths::dataDir / input ).lexically_normal();
}

std::optional< std::uint16_t > parsePort( const std::string& value ) {
	try {
		const auto parsed = std::stoul( value );
		if ( parsed > std::numeric_limits< std::uint16_t >::max() ) {
			return std::nullopt;
		}
		return static_cast< std::uint16_t >( parsed );
	} catch ( const std::exception& ) {
		return std::nullopt;
	}
}

void printHelp() {
	std::cout << "Usage: task1_server [--port PORT] [--data FILE_OR_PATH]\n";
	std::cout << "Default data file: " << defaultDataFile().string() << '\n';
	std::cout << "Relative values passed to --data are resolved inside: " << paths::dataDir.string() << '\n';
}

}  // namespace

ServerConsoleApp::ServerConsoleApp( std::span< char* const > args )
    : args_( args ), data_file_path_( defaultDataFile() ) {}

int ServerConsoleApp::run() {
	parseArguments();
	if ( shouldPrintHelp() ) {
		printHelp();
		return 0;
	}

	const auto seed_data = ServerSeedDataLoader::loadFromFile( data_file_path_ );

	registerSignalHandlers();
	keepRunningFlag() = 1;

	TicketServer server(
	    seed_data.tickets, seed_data.cashbox,
	    std::chrono::seconds( 60 ) );  // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
	TicketServerHost host( server, port_ );

	host.start();
	std::cout << "Ticket server is listening on 127.0.0.1:" << host.port() << '\n';
	std::cout << "Data file: " << data_file_path_.string() << '\n';
	std::cout << "Start clients in separate terminals, for example:\n";
	std::cout << "  task1 --host 127.0.0.1 --port " << host.port() << '\n';
	std::cout << "Press Ctrl+C to stop the server.\n";

	while ( keepRunningFlag() != 0 ) {
		std::this_thread::sleep_for( std::chrono::milliseconds(
		    200 ) );  // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
	}

	std::cout << "Stopping server...\n";
	host.stop();
	return 0;
}

void ServerConsoleApp::parseArguments() {
	for ( std::size_t index = 1; index < args_.size(); ++index ) {
		const std::string_view argument = args_[ index ];
		if ( argument == "--help" || argument == "-h" ) {
			show_help_ = true;
			return;
		}

		if ( argument == "--port" ) {
			if ( index + 1 >= args_.size() ) {
				throw std::runtime_error( "Missing value after --port" );
			}

			auto parsed_port = parsePort( args_[ ++index ] );
			if ( !parsed_port.has_value() ) {
				throw std::runtime_error( "Invalid port value" );
			}
			port_ = *parsed_port;
			continue;
		}

		if ( argument == "--data" ) {
			if ( index + 1 >= args_.size() ) {
				throw std::runtime_error( "Missing value after --data" );
			}

			data_file_path_ = resolveDataFilePath( args_[ ++index ] );
			continue;
		}

		throw std::runtime_error( "Unknown argument: " + std::string( argument ) );
	}
}

bool ServerConsoleApp::shouldPrintHelp() const noexcept {
	return show_help_;
}

void ServerConsoleApp::registerSignalHandlers() {
	std::signal( SIGINT, &ServerConsoleApp::onSignal );
}

void ServerConsoleApp::onSignal( int /*unused*/ ) {
	keepRunningFlag() = 0;
}

}  // namespace task1