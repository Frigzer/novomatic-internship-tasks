#pragma once

#include <cstdint>
#include <filesystem>

namespace task1 {

class ServerConsoleApp {
public:
    ServerConsoleApp(int argc, char* argv[]);

    int run();

private:
    void parseArguments();
    [[nodiscard]] bool shouldPrintHelp() const noexcept;
    void printHelp() const;
    static void registerSignalHandlers();
    static void onSignal(int signal_number);

    int argc_{0};
    char** argv_{nullptr};
    std::uint16_t port_{5555};
    std::filesystem::path data_file_path_{};
    bool show_help_{false};
};

}  // namespace task1
