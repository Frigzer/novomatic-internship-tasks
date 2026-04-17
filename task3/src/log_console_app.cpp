#include "log_console_app.hpp"

#include "log_query_engine.hpp"
#include "log_store.hpp"

#include <exception>
#include <string_view>
#include <vector>

namespace task3 {

LogConsoleApp::LogConsoleApp( std::ostream& output, std::ostream& error ) : output_( output ), error_( error ) {}

std::vector< std::string > LogConsoleApp::makeArguments( int argc, char** argv ) {
	std::vector< std::string > args;
	args.reserve( static_cast< std::size_t >( argc ) );
	for ( int i = 0; i < argc; ++i ) {
		args.emplace_back( argv[ i ] );
	}
	return args;
}

void LogConsoleApp::printEntries( std::ostream& out, std::span< const LogEntry* const > entries ) {
	for ( const LogEntry* entry : entries ) {
		out << entry->raw_line << '\n';
	}
}

int LogConsoleApp::execute( const CliOptions& options ) {
	LogStore store;
	const std::filesystem::path resolvedLogFile = fileResolver_.resolve( options.logFile );
	store.loadFromFile( resolvedLogFile );

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
	const std::string_view executableName = args.empty() ? "task3" : std::string_view( args.front() );
	const CliParseResult parseResult      = parser_.parse( args );

	switch ( parseResult.mode ) {
	case CliMode::ShowHelp: output_ << parser_.usage( executableName ); return 0;
	case CliMode::Error:
		error_ << "Error: " << parseResult.message << '\n';
		error_ << parser_.usage( executableName );
		return 1;
	case CliMode::Execute: break;
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
