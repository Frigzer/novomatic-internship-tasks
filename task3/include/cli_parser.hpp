#pragma once

#include "cli_options.hpp"

#include <span>
#include <string>
#include <string_view>

namespace task3::CliParser {

enum class CliMode : std::uint8_t { Execute, ShowHelp, Error };

struct CliParseResult {
	CliMode mode = CliMode::Error;
	CliOptions options;
	std::string message;
};

[[nodiscard]] CliParseResult parse( std::span< const std::string > args );
[[nodiscard]] std::string usage( std::string_view executableName );

[[nodiscard]] static std::string requireValue( std::span< const std::string > args, std::size_t& index,
                                               std::string_view option );

}  // namespace task3::CliParser
