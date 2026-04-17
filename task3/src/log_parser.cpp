#include "log_parser.hpp"

#include <charconv>
#include <chrono>
#include <fstream>
#include <stdexcept>

namespace task3 {
namespace {

std::string extractBracketedToken( const std::string& line, std::size_t& pos ) {
	if ( pos >= line.size() || line[ pos ] != '[' ) {
		return {};
	}

	const std::size_t end = line.find( ']', pos );
	if ( end == std::string::npos ) {
		return {};
	}

	std::string token = line.substr( pos + 1, end - pos - 1 );
	pos               = end + 1;

	if ( pos < line.size() && line[ pos ] == ' ' ) {
		++pos;
	}

	return token;
}

int parseInt( std::string_view text ) {
	int value         = 0;
	const auto* begin = text.data();
	const auto* end   = text.data() + text.size();

	const auto [ ptr, ec ] = std::from_chars( begin, end, value );
	if ( ec != std::errc{} || ptr != end ) {
		throw std::runtime_error( "Invalid integer value: " + std::string( text ) );
	}

	return value;
}

}  // namespace

std::string_view toString( LogLevel level ) {
	switch ( level ) {
	case LogLevel::Trace: return "TRACE";
	case LogLevel::Debug: return "DEBUG";
	case LogLevel::Info: return "INFO";
	case LogLevel::Warn: return "WARN";
	case LogLevel::Error: return "ERROR";
	case LogLevel::Fatal: return "FATAL";
	}

	return "UNKNOWN";
}

LogLevel parseLogLevel( std::string_view text ) {
	if ( text == "TRACE" ) return LogLevel::Trace;
	if ( text == "DEBUG" ) return LogLevel::Debug;
	if ( text == "INFO" ) return LogLevel::Info;
	if ( text == "WARN" ) return LogLevel::Warn;
	if ( text == "ERROR" ) return LogLevel::Error;
	if ( text == "FATAL" ) return LogLevel::Fatal;

	throw std::runtime_error( "Unknown log level: " + std::string( text ) );
}

std::chrono::sys_seconds LogParser::parseTimestamp( const std::string& text ) {
	// Oczekiwany format: YYYY-MM-DDTHH:MM:SS
	if ( text.size() != 19 || text[ 4 ] != '-' || text[ 7 ] != '-' || text[ 10 ] != 'T' || text[ 13 ] != ':' ||
	     text[ 16 ] != ':' ) {
		throw std::runtime_error( "Invalid timestamp format: " + text );
	}

	const int parsedYear       = parseInt( std::string_view( text ).substr( 0, 4 ) );
	const unsigned parsedMonth = static_cast< unsigned >( parseInt( std::string_view( text ).substr( 5, 2 ) ) );
	const unsigned parsedDay   = static_cast< unsigned >( parseInt( std::string_view( text ).substr( 8, 2 ) ) );
	const int parsedHour       = parseInt( std::string_view( text ).substr( 11, 2 ) );
	const int parsedMinute     = parseInt( std::string_view( text ).substr( 14, 2 ) );
	const int parsedSecond     = parseInt( std::string_view( text ).substr( 17, 2 ) );

	using namespace std::chrono;

	const year_month_day ymd{ year{ parsedYear }, month{ parsedMonth }, day{ parsedDay } };

	if ( !ymd.ok() ) {
		throw std::runtime_error( "Invalid calendar date: " + text );
	}

	if ( parsedHour < 0 || parsedHour > 23 || parsedMinute < 0 || parsedMinute > 59 || parsedSecond < 0 ||
	     parsedSecond > 59 ) {
		throw std::runtime_error( "Invalid time value: " + text );
	}

	return sys_days{ ymd } + hours{ parsedHour } + minutes{ parsedMinute } + seconds{ parsedSecond };
}

std::optional< LogEntry > LogParser::parseLine( const std::string& line ) {
	if ( line.empty() ) {
		return std::nullopt;
	}

	std::size_t pos = 0;

	const std::string timestampToken = extractBracketedToken( line, pos );
	const std::string levelToken     = extractBracketedToken( line, pos );
	const std::string sourceToken    = extractBracketedToken( line, pos );

	if ( timestampToken.empty() || levelToken.empty() || sourceToken.empty() ) {
		return std::nullopt;
	}

	if ( pos > line.size() ) {
		return std::nullopt;
	}

	const std::string message = line.substr( pos );
	if ( message.empty() ) {
		return std::nullopt;
	}

	LogEntry entry;
	entry.timestamp = parseTimestamp( timestampToken );
	entry.level     = parseLogLevel( levelToken );
	entry.source    = sourceToken;
	entry.message   = message;
	entry.raw_line  = line;

	return entry;
}

std::vector< LogEntry > LogParser::parseFile( const std::filesystem::path& path ) {
	std::ifstream input( path );
	if ( !input ) {
		throw std::runtime_error( "Failed to open log file: " + path.string() );
	}

	std::vector< LogEntry > entries;
	std::string line;
	std::size_t lineNumber = 0;

	while ( std::getline( input, line ) ) {
		++lineNumber;

		if ( line.empty() ) {
			continue;
		}

		try {
			auto parsed = parseLine( line );
			if ( !parsed.has_value() ) {
				throw std::runtime_error( "Invalid log line format" );
			}

			entries.push_back( std::move( *parsed ) );
		} catch ( const std::exception& ex ) {
			throw std::runtime_error( "Error while parsing line " + std::to_string( lineNumber ) + " in file '" +
			                          path.string() + "': " + ex.what() );
		}
	}

	return entries;
}

}  // namespace task3