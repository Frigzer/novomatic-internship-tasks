#include "blueprint_viewer_app.hpp"

#include <exception>
#include <iostream>

int main() {
	try {
		task2::BlueprintViewerApp app;
		return app.run();
	} catch ( const std::exception& ex ) {
		std::cerr << "Fatal error: " << ex.what() << '\n';
		return 1;
	}
}