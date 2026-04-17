#include "log_parser.hpp"

#include <cctype>
#include <charconv>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace task3 {
namespace {

[[nodiscard]] std::string_view trimView( std::string_view text ) noexcept {
	while ( !text.empty() && std::isspace( static_cast< unsigned char >( text.front() ) ) != 0 ) {
		text.remove_prefix( 1 );
	}

	while ( !text.empty() && std::isspace( static_cast< unsigned char >( text.back() ) ) != 0 ) {
		text.remove_suffix( 1 );
	}

	return text;
}

[[nodiscard]] std::string_view trimTrailingCarriageReturn( std::string_view text ) noexcept {
	if ( !text.empty() && text.back() == '\r' ) {
		text.remove_suffix( 1 );
	}
	return text;
}

[[nodiscard]] std::optional< std::string_view > extractBracketedToken( std::string_view line, std::size_t& pos ) {
	if ( pos >= line.size() || line[ pos ] != '[' ) {
		return std::nullopt;
	}

	const std::size_t end = line.find( ']', pos );
	if ( end == std::string_view::npos ) {
		return std::nullopt;
	}

	const std::size_t tokenBegin = pos + 1;
	pos                          = end + 1;

	if ( pos < line.size() && line[ pos ] == ' ' ) {
		++pos;
	}

	return line.substr( tokenBegin, end - tokenBegin );
}

[[nodiscard]] int parseInt( std::string_view text ) {
	int value         = 0;
	const char* begin = text.data();
	const char* end   = text.data() + text.size();

	const auto [ ptr, ec ] = std::from_chars( begin, end, value );
	if ( ec != std::errc{} || ptr != end ) {
		throw std::runtime_error( "Invalid integer value: " + std::string( text ) );
	}

	return value;
}

}  // namespace

std::string_view toString( LogLevel level ) noexcept {
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

std::chrono::sys_seconds LogParser::parseTimestamp( std::string_view text ) {
	if ( text.size() != 19 || text[ 4 ] != '-' || text[ 7 ] != '-' || text[ 10 ] != 'T' || text[ 13 ] != ':' ||
	     text[ 16 ] != ':' ) {
		throw std::runtime_error( "Invalid timestamp format: " + std::string( text ) );
	}

	const int parsedYear       = parseInt( text.substr( 0, 4 ) );
	const unsigned parsedMonth = static_cast< unsigned >( parseInt( text.substr( 5, 2 ) ) );
	const unsigned parsedDay   = static_cast< unsigned >( parseInt( text.substr( 8, 2 ) ) );
	const int parsedHour       = parseInt( text.substr( 11, 2 ) );
	const int parsedMinute     = parseInt( text.substr( 14, 2 ) );
	const int parsedSecond     = parseInt( text.substr( 17, 2 ) );

	using namespace std::chrono;

	const year_month_day ymd{ year{ parsedYear }, month{ parsedMonth }, day{ parsedDay } };
	if ( !ymd.ok() ) {
		throw std::runtime_error( "Invalid calendar date: " + std::string( text ) );
	}

	if ( parsedHour < 0 || parsedHour > 23 || parsedMinute < 0 || parsedMinute > 59 || parsedSecond < 0 ||
	     parsedSecond > 59 ) {
		throw std::runtime_error( "Invalid time value: " + std::string( text ) );
	}

	return sys_days{ ymd } + hours{ parsedHour } + minutes{ parsedMinute } + seconds{ parsedSecond };
}

std::optional< LogEntry > LogParser::parseLine( std::string_view line ) {
	line = trimTrailingCarriageReturn( line );
	if ( trimView( line ).empty() ) {
		return std::nullopt;
	}

	std::size_t pos = 0;

	const auto timestampToken = extractBracketedToken( line, pos );
	const auto levelToken     = extractBracketedToken( line, pos );
	const auto sourceToken    = extractBracketedToken( line, pos );

	if ( !timestampToken.has_value() || !levelToken.has_value() || !sourceToken.has_value() ) {
		return std::nullopt;
	}

	if ( pos > line.size() ) {
		return std::nullopt;
	}

	const std::string_view messageView = line.substr( pos );
	if ( trimView( messageView ).empty() ) {
		return std::nullopt;
	}

	LogEntry entry;
	entry.timestamp = parseTimestamp( *timestampToken );
	entry.level     = parseLogLevel( *levelToken );
	entry.source    = std::string( *sourceToken );
	entry.message   = std::string( messageView );
	entry.raw_line  = std::string( line );

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

		try {
			auto parsed = parseLine( line );
			if ( !parsed.has_value() ) {
				continue;
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
