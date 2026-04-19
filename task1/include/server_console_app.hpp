#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

namespace task1 {

class ServerConsoleApp {
public:
	explicit ServerConsoleApp( std::span< char* const > args );

	int run();

private:
	static constexpr std::uint16_t defaultPort = 5555;

	void parseArguments();
	[[nodiscard]] bool shouldPrintHelp() const noexcept;
	static void registerSignalHandlers();
	static void onSignal( int signal_number );

	std::span< char* const > args_;
	std::uint16_t port_{ defaultPort };
	std::filesystem::path data_file_path_{};
	bool show_help_{ false };
};

}  // namespace task1