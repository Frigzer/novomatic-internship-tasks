#include "server_console_app.hpp"

#include <exception>
#include <iostream>

int main( int argc, char* argv[] ) {
	try {
		task1::ServerConsoleApp app( argc, argv );
		return app.run();
	} catch ( const std::exception& exception ) {
		std::cerr << "Server error: " << exception.what() << '\n';
		return 1;
	}
}
