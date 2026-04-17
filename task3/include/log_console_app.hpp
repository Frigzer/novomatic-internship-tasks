#include <chrono>
#include <iostream>
#include <optional>
#include <vector>
#pragma once

#include "cli_options.hpp"
#include "cli_parser.hpp"
#include "log_file_resolver.hpp"

#include <istream>
#include <ostream>
#include <span>
#include <string>

namespace task3 {

class LogConsoleApp {
public:
	explicit LogConsoleApp( std::istream& input = std::cin, std::ostream& output = std::cout,
	                        std::ostream& error = std::cerr );

	int run( int argc, char** argv );

private:
	[[nodiscard]] int runWithArguments( std::span< const std::string > args );
	[[nodiscard]] int runInteractive( std::string_view executableName );
	[[nodiscard]] int execute( const CliOptions& options );

	[[nodiscard]] bool promptYesNo( std::string_view label, bool defaultValue ) const;
	[[nodiscard]] std::string promptLine( std::string_view label ) const;
	[[nodiscard]] std::optional< std::chrono::sys_seconds > promptOptionalTimestamp( std::string_view label ) const;
	[[nodiscard]] std::optional< LogLevel > promptOptionalLogLevel( std::string_view label ) const;

	static std::vector< std::string > makeArguments( int argc, char** argv );
	static void printEntries( std::ostream& out, std::span< const LogEntry* const > entries );

	std::istream& input_;
	std::ostream& output_;
	std::ostream& error_;
	CliParser parser_;
	LogFileResolver fileResolver_;
};

}  // namespace task3
