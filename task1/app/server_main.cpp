#include "server_console_app.hpp"

#include <exception>
#include <iostream>
#include <span>

int main( int argc, char* argv[] ) {
	try {
		task1::ServerConsoleApp app( std::span< char* const >{ argv, static_cast< std::size_t >( argc ) } );
		return app.run();
	} catch ( const std::exception& exception ) {
		std::cerr << "Server error: " << exception.what() << '\n';
		return 1;
	}
}