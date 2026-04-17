#include "log_file_resolver.hpp"

#include "paths.hpp"

#include <system_error>
#include <vector>

namespace task3 {

bool LogFileResolver::exists( const std::filesystem::path& candidate ) noexcept {
	std::error_code errorCode;
	return std::filesystem::exists( candidate, errorCode );
}

std::filesystem::path LogFileResolver::normalized( const std::filesystem::path& candidate ) {
	std::error_code errorCode;
	const std::filesystem::path normalizedPath = std::filesystem::weakly_canonical( candidate, errorCode );
	if ( !errorCode ) {
		return normalizedPath;
	}
	return candidate.lexically_normal();
}

bool LogFileResolver::isPlainFilename( const std::filesystem::path& input ) noexcept {
	return !input.empty() && !input.has_parent_path();
}

std::filesystem::path LogFileResolver::resolve( const std::filesystem::path& input ) const {
	if ( input.empty() ) {
		return input;
	}

	if ( input.is_absolute() ) {
		return normalized( input );
	}

	std::error_code errorCode;
	const std::filesystem::path currentDirectory = std::filesystem::current_path( errorCode );

	std::vector< std::filesystem::path > candidates;
	candidates.reserve( 5 );

	if ( !errorCode ) {
		candidates.push_back( currentDirectory / input );
		if ( isPlainFilename( input ) ) {
			candidates.push_back( currentDirectory / "data" / input.filename() );
		}
	}

	candidates.push_back( paths::sourceDir / input );
	if ( isPlainFilename( input ) ) {
		candidates.push_back( paths::dataDir / input.filename() );
	} else {
		candidates.push_back( paths::dataDir / input );
	}

	for ( const auto& candidate : candidates ) {
		if ( exists( candidate ) ) {
			return normalized( candidate );
		}
	}

	if ( !errorCode ) {
		return normalized( currentDirectory / input );
	}

	return input.lexically_normal();
}

}  // namespace task3
