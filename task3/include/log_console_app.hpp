#pragma once

#include "cli_options.hpp"
#include "log_entry.hpp"

#include <iostream>
#include <ostream>
#include <span>
#include <string>
#include <vector>

namespace task3 {

class LogConsoleApp {
public:
	explicit LogConsoleApp( std::ostream& output = std::cout, std::ostream& error = std::cerr );

	int run( int argc, char** argv );

private:
	[[nodiscard]] int runWithArguments( std::span< const std::string > args );
	[[nodiscard]] int execute( const CliOptions& options );

	static std::vector< std::string > makeArguments( int argc, char** argv );
	static void printEntries( std::ostream& out, std::span< const LogEntry* const > entries );

	std::ostream& output_;
	std::ostream& error_;
};

}  // namespace task3
