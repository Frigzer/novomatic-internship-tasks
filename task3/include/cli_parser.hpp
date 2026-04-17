#pragma once

#include "cli_options.hpp"

#include <span>
#include <string>
#include <string_view>

namespace task3 {

enum class CliMode { Execute, Interactive, ShowHelp, Error };

struct CliParseResult {
	CliMode mode = CliMode::Error;
	CliOptions options;
	std::string message;
};

class CliParser {
public:
	[[nodiscard]] CliParseResult parse( std::span< const std::string > args ) const;
	[[nodiscard]] std::string usage( std::string_view executableName ) const;

private:
	[[nodiscard]] static std::string requireValue( std::span< const std::string > args, std::size_t& index,
	                                               std::string_view option );
};

}  // namespace task3
