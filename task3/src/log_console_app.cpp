#include "log_console_app.hpp"

#include "log_parser.hpp"
#include "log_query_engine.hpp"
#include "log_store.hpp"

#include <exception>
#include <iostream>
#include <limits>
#include <optional>
#include <string_view>
#include <vector>

namespace task3 {

LogConsoleApp::LogConsoleApp( std::istream& input, std::ostream& output, std::ostream& error )
    : input_( input ), output_( output ), error_( error ) {}

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

bool LogConsoleApp::promptYesNo( std::string_view label, bool defaultValue ) const {
	while ( true ) {
		output_ << label << ( defaultValue ? " [Y/n]: " : " [y/N]: " );

		std::string line;
		if ( !std::getline( input_, line ) ) {
			return defaultValue;
		}

		if ( line.empty() ) {
			return defaultValue;
		}

		if ( line == "y" || line == "Y" || line == "yes" || line == "YES" ) {
			return true;
		}

		if ( line == "n" || line == "N" || line == "no" || line == "NO" ) {
			return false;
		}

		error_ << "Please answer y or n.\n";
	}
}

std::string LogConsoleApp::promptLine( std::string_view label ) const {
	output_ << label;
	std::string line;
	std::getline( input_, line );
	return line;
}

std::optional< std::chrono::sys_seconds > LogConsoleApp::promptOptionalTimestamp( std::string_view label ) const {
	while ( true ) {
		const std::string value = promptLine( label );
		if ( value.empty() ) {
			return std::nullopt;
		}

		try {
			return LogParser::parseTimestamp( value );
		} catch ( const std::exception& ex ) {
			error_ << "Invalid timestamp: " << ex.what() << '\n';
		}
	}
}

std::optional< LogLevel > LogConsoleApp::promptOptionalLogLevel( std::string_view label ) const {
	while ( true ) {
		const std::string value = promptLine( label );
		if ( value.empty() ) {
			return std::nullopt;
		}

		try {
			return parseLogLevel( value );
		} catch ( const std::exception& ex ) {
			error_ << "Invalid log level: " << ex.what() << '\n';
		}
	}
}

int LogConsoleApp::execute( const CliOptions& options ) {
	LogStore store;
	store.loadFromFile( options.logFile );

	LogQueryEngine engine( store.view() );
	const auto results = engine.execute( options.query );

	if ( options.countOnly ) {
		output_ << results.size() << '\n';
		return 0;
	}

	printEntries( output_, results );
	return 0;
}

int LogConsoleApp::runInteractive( std::string_view executableName ) {
	output_ << "Advanced Log Analyzer\n";
	output_ << "Press Enter to leave an optional field empty.\n";
	output_ << "Type 'help' as the log file path to see the CLI usage.\n\n";

	CliOptions options;

	while ( options.logFile.empty() ) {
		const std::string path = promptLine( "Log file path: " );
		if ( path == "help" ) {
			output_ << '\n' << parser_.usage( executableName ) << '\n';
			continue;
		}
		if ( path.empty() ) {
			error_ << "Log file path is required.\n";
			continue;
		}
		options.logFile = path;
	}

	options.query.level = promptOptionalLogLevel( "Level [TRACE/DEBUG/INFO/WARN/ERROR/FATAL]: " );

	const std::string source = promptLine( "Source: " );
	if ( !source.empty() ) {
		options.query.source = source;
	}

	const std::string message = promptLine( "Message contains: " );
	if ( !message.empty() ) {
		options.query.messageContains = message;
	}

	const auto exactTimestamp = promptOptionalTimestamp( "Exact timestamp [YYYY-MM-DDTHH:MM:SS]: " );
	if ( exactTimestamp.has_value() ) {
		options.query.timeRange = TimeRange{ *exactTimestamp, *exactTimestamp };
	} else {
		const auto from = promptOptionalTimestamp( "From timestamp [YYYY-MM-DDTHH:MM:SS]: " );
		const auto to   = promptOptionalTimestamp( "To timestamp [YYYY-MM-DDTHH:MM:SS]: " );

		if ( from.has_value() != to.has_value() ) {
			error_ << "Ignoring incomplete time range: both From and To are required.\n";
		} else if ( from.has_value() && to.has_value() ) {
			if ( *from > *to ) {
				error_ << "Ignoring invalid time range: From must be earlier than or equal to To.\n";
			} else {
				options.query.timeRange = TimeRange{ *from, *to };
			}
		}
	}

	options.countOnly = promptYesNo( "Print only the number of matches?", false );
	output_ << '\n';

	try {
		return execute( options );
	} catch ( const std::exception& ex ) {
		error_ << "Error: " << ex.what() << '\n';
		return 1;
	}
}

int LogConsoleApp::runWithArguments( std::span< const std::string > args ) {
	const std::string_view executableName = args.empty() ? "task3" : std::string_view( args.front() );
	const CliParseResult parseResult      = parser_.parse( args );

	switch ( parseResult.mode ) {
	case CliMode::Interactive: return runInteractive( executableName );
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
