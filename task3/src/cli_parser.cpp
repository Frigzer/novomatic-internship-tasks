#include "cli_parser.hpp"

#include "log_parser.hpp"

#include <optional>
#include <stdexcept>

namespace task3::CliParser {

namespace {

struct RawTimeArgs {
	std::optional< std::chrono::sys_seconds > from;
	std::optional< std::chrono::sys_seconds > to;
	std::optional< std::chrono::sys_seconds > exact;
};

std::string requireValue( std::span< const std::string > args, std::size_t& index, std::string_view option ) {
	if ( index + 1 >= args.size() ) {
		throw std::runtime_error( "Missing value for option " + std::string( option ) );
	}
	return args[ ++index ];
}

void handleOption( const std::string& arg, std::span< const std::string > args, std::size_t& index, CliOptions& opt,
                   RawTimeArgs& times ) {
	if ( arg == "--level" )
		opt.query.level = parseLogLevel( requireValue( args, index, arg ) );
	else if ( arg == "--source" )
		opt.query.source = requireValue( args, index, arg );
	else if ( arg == "--message" )
		opt.query.messageContains = requireValue( args, index, arg );
	else if ( arg == "--case-sensitive" )
		opt.query.messageCaseSensitive = true;
	else if ( arg == "--count" )
		opt.countOnly = true;
	else if ( arg == "--from" )
		times.from = LogParser::parseTimestamp( requireValue( args, index, arg ) );
	else if ( arg == "--to" )
		times.to = LogParser::parseTimestamp( requireValue( args, index, arg ) );
	else if ( arg == "--timestamp" )
		times.exact = LogParser::parseTimestamp( requireValue( args, index, arg ) );
	else
		throw std::runtime_error( "Unknown option: " + arg );
}

void validateAndFinalize( CliOptions& opt, const RawTimeArgs& times ) {
	if ( opt.logFile.empty() ) {
		throw std::runtime_error( "Missing log file path." );
	}

	if ( opt.query.messageCaseSensitive && !opt.query.messageContains.has_value() ) {
		throw std::runtime_error( "Option --case-sensitive requires --message." );
	}

	// Walidacja logiki czasu
	if ( times.exact.has_value() && ( times.from.has_value() || times.to.has_value() ) ) {
		throw std::runtime_error( "Use either --timestamp or --from/--to, not both." );
	}

	if ( times.exact.has_value() ) {
		opt.query.timeRange = TimeRange{ .from = *times.exact, .to = *times.exact };
	} else if ( times.from.has_value() || times.to.has_value() ) {
		if ( !times.from.has_value() || !times.to.has_value() ) {
			throw std::runtime_error( "Both --from and --to must be provided together." );
		}
		if ( *times.from > *times.to ) {
			throw std::runtime_error( "Invalid time range: --from must be earlier than --to." );
		}
		opt.query.timeRange = TimeRange{ .from = *times.from, .to = *times.to };
	}
}
}  // namespace

std::string usage( std::string_view executableName ) {
	std::string text;
	text += "Usage:\n";
	text += "  ";
	text += executableName;
	text += " <log-file> [options]\n\n";
	text += "The log file may be an absolute path, a relative path, 'data/<file>', or just a file name\n";
	text += "when the file exists in the current directory, the current data/ directory, or the project's data "
	        "directory.\n\n";
	text += "Options:\n";
	text += "  --level <TRACE|DEBUG|INFO|WARN|ERROR|FATAL>\n";
	text += "  --source <source>\n";
	text += "  --message <substring>                 Case-insensitive by default\n";
	text += "  --case-sensitive                      Make --message matching case-sensitive\n";
	text += "  --from <YYYY-MM-DDTHH:MM:SS>\n";
	text += "  --to <YYYY-MM-DDTHH:MM:SS>\n";
	text += "  --timestamp <YYYY-MM-DDTHH:MM:SS>   Exact timestamp match\n";
	text += "  --count                              Print only the number of matches\n";
	text += "  --help                               Show this help\n\n";
	text += "Examples:\n";
	text += "  ";
	text += executableName;
	text += " sample_logs.txt --source AuthService\n";
	text += "  ";
	text += executableName;
	text += " data/sample_logs.txt --level ERROR --from 2023-10-25T10:00:00 --to 2023-10-25T10:10:00\n";
	text += "  ";
	text += executableName;
	text += " sample_logs.txt --message Transaction\n";
	text += "  ";
	text += executableName;
	text += " sample_logs.txt --message Timeout --case-sensitive\n";
	return text;
}

CliParseResult parse( std::span< const std::string > args ) {
	if ( args.size() <= 1 ) {
		return { .mode = CliMode::Error, .message = "Missing log file path." };
	}

	try {
		CliOptions options;
		RawTimeArgs times;

		for ( std::size_t i = 1; i < args.size(); ++i ) {
			const std::string& arg = args[ i ];

			if ( arg == "--help" ) return { .mode = CliMode::ShowHelp };

			if ( !arg.starts_with( "--" ) ) {
				if ( !options.logFile.empty() ) {
					throw std::runtime_error( "Multiple log file paths provided: '" + options.logFile.string() +
					                          "' and '" + arg + "'" );
				}
				options.logFile = arg;
			} else {
				handleOption( arg, args, i, options, times );
			}
		}

		validateAndFinalize( options, times );
		return { .mode = CliMode::Execute, .options = std::move( options ) };

	} catch ( const std::exception& ex ) {
		return { .mode = CliMode::Error, .message = ex.what() };
	}
}

}  // namespace task3::CliParser
