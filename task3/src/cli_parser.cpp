#include "cli_parser.hpp"

#include "log_parser.hpp"

#include <optional>
#include <stdexcept>

namespace task3 {

std::string CliParser::usage( std::string_view executableName ) const {
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

std::string CliParser::requireValue( std::span< const std::string > args, std::size_t& index,
                                     std::string_view option ) {
	if ( index + 1 >= args.size() ) {
		throw std::runtime_error( "Missing value for option " + std::string( option ) );
	}
	return args[ ++index ];
}

CliParseResult CliParser::parse( std::span< const std::string > args ) const {
	CliParseResult result;

	if ( args.size() <= 1 ) {
		result.mode    = CliMode::Error;
		result.message = "Missing log file path.";
		return result;
	}

	try {
		CliOptions options;
		std::optional< std::chrono::sys_seconds > from;
		std::optional< std::chrono::sys_seconds > to;
		std::optional< std::chrono::sys_seconds > exactTimestamp;

		for ( std::size_t index = 1; index < args.size(); ++index ) {
			const std::string& arg = args[ index ];

			if ( arg == "--help" ) {
				result.mode = CliMode::ShowHelp;
				return result;
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
				options.query.level = parseLogLevel( requireValue( args, index, arg ) );
			} else if ( arg == "--source" ) {
				options.query.source = requireValue( args, index, arg );
			} else if ( arg == "--message" ) {
				options.query.messageContains = requireValue( args, index, arg );
			} else if ( arg == "--case-sensitive" ) {
				options.query.messageCaseSensitive = true;
			} else if ( arg == "--from" ) {
				from = LogParser::parseTimestamp( requireValue( args, index, arg ) );
			} else if ( arg == "--to" ) {
				to = LogParser::parseTimestamp( requireValue( args, index, arg ) );
			} else if ( arg == "--timestamp" ) {
				exactTimestamp = LogParser::parseTimestamp( requireValue( args, index, arg ) );
			} else if ( arg == "--count" ) {
				options.countOnly = true;
			} else {
				throw std::runtime_error( "Unknown option: " + arg );
			}
		}

		if ( options.logFile.empty() ) {
			throw std::runtime_error( "Missing log file path." );
		}

		if ( options.query.messageCaseSensitive && !options.query.messageContains.has_value() ) {
			throw std::runtime_error( "Option --case-sensitive requires --message." );
		}

		if ( exactTimestamp.has_value() && ( from.has_value() || to.has_value() ) ) {
			throw std::runtime_error( "Use either --timestamp or --from/--to, not both." );
		}

		if ( exactTimestamp.has_value() ) {
			options.query.timeRange = TimeRange{ *exactTimestamp, *exactTimestamp };
		} else if ( from.has_value() || to.has_value() ) {
			if ( !from.has_value() || !to.has_value() ) {
				throw std::runtime_error( "Both --from and --to must be provided together." );
			}
			if ( *from > *to ) {
				throw std::runtime_error( "Invalid time range: --from must be earlier than or equal to --to." );
			}
			options.query.timeRange = TimeRange{ *from, *to };
		}

		result.mode    = CliMode::Execute;
		result.options = std::move( options );
		return result;
	} catch ( const std::exception& ex ) {
		result.mode    = CliMode::Error;
		result.message = ex.what();
		return result;
	}
}

}  // namespace task3
