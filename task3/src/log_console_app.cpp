#include "log_console_app.hpp"

#include "cli_parser.hpp"
#include "log_file_resolver.hpp"
#include "log_query_engine.hpp"
#include "log_store.hpp"

#include <exception>
#include <ranges>
#include <sstream>
#include <string_view>
#include <vector>

namespace task3 {

namespace {

std::string buildMissingFileMessage( const LogFileResolver::FileResolutionResult& resolution,
                                     const std::filesystem::path& requestedPath ) {
	std::ostringstream message;
	message << "Could not find log file '" << requestedPath.string() << "'.";

	if ( !resolution.attempted.empty() ) {
		message << " Tried:";
		for ( const auto& candidate : resolution.attempted ) {
			message << "\n  - " << candidate.string();
		}
	}

	return message.str();
}
}  // namespace

LogConsoleApp::LogConsoleApp( std::ostream& output, std::ostream& error ) : output_( output ), error_( error ) {}

std::vector< std::string > LogConsoleApp::makeArguments( int argc, char** argv ) {
	if ( argc <= 0 || argv == nullptr ) {
		return {};
	}

	return std::views::counted( argv, argc ) | std::ranges::to< std::vector< std::string > >();
}

void LogConsoleApp::printEntries( std::ostream& out, std::span< const LogEntry* const > entries ) {
	for ( const LogEntry* entry : entries ) {
		out << entry->raw_line << '\n';
	}
}

int LogConsoleApp::execute( const CliOptions& options ) {
	LogStore store;
	const LogFileResolver::FileResolutionResult resolution = LogFileResolver::resolveDetailed( options.logFile );
	if ( !resolution.exists ) {
		throw std::runtime_error( buildMissingFileMessage( resolution, options.logFile ) );
	}
	store.loadFromFile( resolution.resolved );

	LogQueryEngine engine( store.view() );
	const auto results = engine.execute( options.query );

	if ( options.countOnly ) {
		output_ << results.size() << '\n';
		return 0;
	}

	printEntries( output_, results );
	return 0;
}

int LogConsoleApp::runWithArguments( std::span< const std::string > args ) {
	const std::string_view executableName       = args.empty() ? "task3" : std::string_view( args.front() );
	const CliParser::CliParseResult parseResult = CliParser::parse( args );

	switch ( parseResult.mode ) {
	case CliParser::CliMode::ShowHelp: output_ << CliParser::usage( executableName ); return 0;
	case CliParser::CliMode::Error:
		error_ << "Error: " << parseResult.message << '\n';
		error_ << CliParser::usage( executableName );
		return 1;
	case CliParser::CliMode::Execute: break;
	}

	try {
		return execute( parseResult.options );
	} catch ( const std::exception& ex ) {
		error_ << "Error: " << ex.what() << '\n';
		return 1;
	}
}

int LogConsoleApp::run( int argc, char** argv ) {
	return runWithArguments( makeArguments( argc, argv ) );
}

}  // namespace task3
