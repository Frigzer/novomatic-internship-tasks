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

void LogFileResolver::appendCandidate( std::vector< std::filesystem::path >& candidates,
                                       const std::filesystem::path& candidate ) {
	if ( std::ranges::find( candidates, candidate ) == candidates.end() ) {
		candidates.push_back( candidate );
	}
}

FileResolutionResult LogFileResolver::resolveDetailed( const std::filesystem::path& input ) const {
	FileResolutionResult result;

	if ( input.empty() ) {
		result.resolved = input;
		return result;
	}

	if ( input.is_absolute() ) {
		result.resolved = normalized( input );
		result.attempted.push_back( result.resolved );
		result.exists = exists( input );
		return result;
	}

	std::error_code errorCode;
	const std::filesystem::path currentDirectory = std::filesystem::current_path( errorCode );

	std::vector< std::filesystem::path > candidates;
	candidates.reserve( 5 );

	if ( !errorCode ) {
		appendCandidate( candidates, currentDirectory / input );
		if ( isPlainFilename( input ) ) {
			appendCandidate( candidates, currentDirectory / "data" / input.filename() );
		}
	}

	appendCandidate( candidates, paths::sourceDir / input );
	if ( isPlainFilename( input ) ) {
		appendCandidate( candidates, paths::dataDir / input.filename() );
	} else {
		appendCandidate( candidates, paths::dataDir / input );
	}

	for ( const auto& candidate : candidates ) {
		const std::filesystem::path normalizedCandidate = normalized( candidate );
		result.attempted.push_back( normalizedCandidate );
		if ( exists( candidate ) ) {
			result.resolved = normalizedCandidate;
			result.exists   = true;
			return result;
		}
	}

	if ( !errorCode ) {
		result.resolved = normalized( currentDirectory / input );
	} else {
		result.resolved = input.lexically_normal();
	}

	return result;
}

std::filesystem::path LogFileResolver::resolve( const std::filesystem::path& input ) const {
	return resolveDetailed( input ).resolved;
}

}  // namespace task3
