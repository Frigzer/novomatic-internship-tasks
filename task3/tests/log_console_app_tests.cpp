#include <gtest/gtest.h>

#include "log_console_app.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace task3 {
namespace {

std::vector< char* > makeArgv( std::vector< std::string >& args ) {
	std::vector< char* > argv;
	argv.reserve( args.size() );
	for ( std::string& arg : args ) {
		argv.push_back( arg.data() );
	}
	return argv;
}

TEST( LogConsoleAppTests, ReportsAttemptedPathsWhenLogFileCannotBeFound ) {
	std::ostringstream output;
	std::ostringstream error;
	LogConsoleApp app( output, error );

	std::vector< std::string > args{ "task3", "definitely_missing_file.txt" };
	std::vector< char* > argv = makeArgv( args );

	const int exitCode = app.run( static_cast< int >( argv.size() ), argv.data() );

	EXPECT_EQ( exitCode, 1 );
	EXPECT_NE( error.str().find( "Could not find log file 'definitely_missing_file.txt'" ), std::string::npos );
	EXPECT_NE( error.str().find( "Tried:" ), std::string::npos );
	EXPECT_NE( error.str().find( "definitely_missing_file.txt" ), std::string::npos );
}

}  // namespace
}  // namespace task3
