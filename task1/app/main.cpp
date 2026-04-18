#include "console_client_app.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        task1::ConsoleClientApp app(argc, argv);
        return app.run();
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << '\n';
        return 1;
    }
}
