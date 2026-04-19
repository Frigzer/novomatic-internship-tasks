#include "paths.hpp"
#include "server_seed_data.hpp"

#include <gtest/gtest.h>

namespace task1 {
namespace {

TEST( ServerSeedDataTests, LoadsDefaultSeedFile ) {
	const auto seed = ServerSeedDataLoader::loadFromFile( paths::dataDir / "server_seed.json" );

	EXPECT_EQ( seed.tickets.size(), 4U );
	EXPECT_EQ( seed.cashbox.total(), 1100 );
}

}  // namespace
}  // namespace task1
