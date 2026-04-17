#include "log_parser.hpp"
#include "log_query_engine.hpp"
#include "log_store.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct CliOptions {
	std::filesystem::path logFile;
	task3::LogQuery query;
	bool countOnly = false;
};

void printUsage( std::ostream& out, std::string_view executableName ) {
	out << "Usage:\n"
	    << "  " << executableName << " <log-file> [options]\n\n"
	    << "Options:\n"
	    << "  --level <TRACE|DEBUG|INFO|WARN|ERROR|FATAL>\n"
	    << "  --source <source>\n"
	    << "  --message <substring>\n"
	    << "  --from <YYYY-MM-DDTHH:MM:SS>\n"
	    << "  --to <YYYY-MM-DDTHH:MM:SS>\n"
	    << "  --timestamp <YYYY-MM-DDTHH:MM:SS>   Exact timestamp match\n"
	    << "  --count                              Print only the number of matches\n"
	    << "  --help                               Show this help\n\n"
	    << "Examples:\n"
	    << "  " << executableName << " logs.txt --source AuthService\n"
	    << "  " << executableName << " logs.txt --level ERROR --from 2023-10-25T10:00:00 --to 2023-10-25T10:10:00\n"
	    << "  " << executableName << " logs.txt --message Transaction\n";
}

[[nodiscard]] std::string requireValue( const std::vector< std::string >& args, std::size_t& index,
                                        std::string_view option ) {
	if ( index + 1 >= args.size() ) {
		throw std::runtime_error( "Missing value for option " + std::string( option ) );
	}
	return args[ ++index ];
}

[[nodiscard]] CliOptions parseArguments( const std::vector< std::string >& args ) {
	if ( args.size() < 2 ) {
		throw std::runtime_error( "Missing log file path. Use --help for usage." );
	}

	CliOptions options;
	std::optional< std::chrono::sys_seconds > from;
	std::optional< std::chrono::sys_seconds > to;
	std::optional< std::chrono::sys_seconds > exactTimestamp;

	for ( std::size_t index = 1; index < args.size(); ++index ) {
		const std::string& arg = args[ index ];

		if ( arg == "--help" ) {
			throw std::runtime_error( "__help__" );
		}

		if ( !arg.starts_with( "--" ) ) {
			if ( !options.logFile.empty() ) {
				throw std::runtime_error( "Multiple log file paths provided: '" + options.logFile.string() +
				                          "' and '" + arg + "'" );
			}
			options.logFile = arg;
			continue;
		}

		if ( arg == "--level" ) {
			options.query.level = task3::parseLogLevel( requireValue( args, index, arg ) );
		} else if ( arg == "--source" ) {
			options.query.source = requireValue( args, index, arg );
		} else if ( arg == "--message" ) {
			options.query.messageContains = requireValue( args, index, arg );
		} else if ( arg == "--from" ) {
			from = task3::LogParser::parseTimestamp( requireValue( args, index, arg ) );
		} else if ( arg == "--to" ) {
			to = task3::LogParser::parseTimestamp( requireValue( args, index, arg ) );
		} else if ( arg == "--timestamp" ) {
			exactTimestamp = task3::LogParser::parseTimestamp( requireValue( args, index, arg ) );
		} else if ( arg == "--count" ) {
			options.countOnly = true;
		} else {
			throw std::runtime_error( "Unknown option: " + arg );
		}
	}

	if ( options.logFile.empty() ) {
		throw std::runtime_error( "Missing log file path. Use --help for usage." );
	}

	if ( exactTimestamp.has_value() && ( from.has_value() || to.has_value() ) ) {
		throw std::runtime_error( "Use either --timestamp or --from/--to, not both." );
	}

	if ( exactTimestamp.has_value() ) {
		options.query.timeRange = task3::TimeRange{ *exactTimestamp, *exactTimestamp };
	} else if ( from.has_value() || to.has_value() ) {
		if ( !from.has_value() || !to.has_value() ) {
			throw std::runtime_error( "Both --from and --to must be provided together." );
		}
		if ( *from > *to ) {
			throw std::runtime_error( "Invalid time range: --from must be earlier than or equal to --to." );
		}
		options.query.timeRange = task3::TimeRange{ *from, *to };
	}

	return options;
}

}  // namespace

int main( int argc, char** argv ) {
	std::vector< std::string > args;
	args.reserve( static_cast< std::size_t >( argc ) );
	for ( int i = 0; i < argc; ++i ) {
		args.emplace_back( argv[ i ] );
	}

	const std::string_view executableName = args.empty() ? "task3" : std::string_view( args.front() );

	try {
		const CliOptions options = parseArguments( args );

		task3::LogStore store;
		store.loadFromFile( options.logFile );

		task3::LogQueryEngine engine( store.view() );
		const auto results = engine.execute( options.query );

		if ( options.countOnly ) {
			std::cout << results.size() << '\n';
			return 0;
		}

		for ( const task3::LogEntry* entry : results ) {
			std::cout << entry->raw_line << '\n';
		}

		return 0;
	} catch ( const std::runtime_error& ex ) {
		if ( std::string_view( ex.what() ) == "__help__" ) {
			printUsage( std::cout, executableName );
			return 0;
		}

		std::cerr << "Error: " << ex.what() << '\n';
		printUsage( std::cerr, executableName );
		return 1;
	} catch ( const std::exception& ex ) {
		std::cerr << "Error: " << ex.what() << '\n';
		return 1;
	}
}
