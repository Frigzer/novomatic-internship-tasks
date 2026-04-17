#include "log_query_engine.hpp"
#include "log_store.hpp"
#include "paths.hpp"

#include <filesystem>
#include <iostream>

int main() {
	try {
		task3::LogStore store;
		store.loadFromFile( task3::paths::dataDir / "sample_logs.txt" );

		task3::LogQuery query;
		query.source = "AuthService";

		task3::LogQueryEngine engine( store.entries() );
		const auto results = engine.execute( query );

		for ( const task3::LogEntry* entry : results ) {
			std::cout << entry->raw_line << '\n';
		}
	} catch ( const std::exception& ex ) {
		std::cerr << "Error: " << ex.what() << '\n';
		return 1;
	}

	return 0;
}