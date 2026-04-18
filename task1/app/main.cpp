#include "console_client_demo_app.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        task1::ConsoleClientDemoApp app;
        return app.run();
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << '\n';
        return 1;
    }
}
