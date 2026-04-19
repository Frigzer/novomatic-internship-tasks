#include <gtest/gtest.h>

#include "change_calculator.hpp"

namespace task1 {

TEST( ChangeCalculatorTests, ReturnsEmptyChangeForZeroAmount ) {
	CoinInventory inventory( { { 200, 3 }, { 100, 2 } } );
	auto result = ChangeCalculator::computeMinimalCoinChange( 0, inventory );

	ASSERT_TRUE( result.has_value() );
	EXPECT_EQ( result->total, 0 );
	EXPECT_TRUE( result->coins.empty() );
}

TEST( ChangeCalculatorTests, FindsSimpleChange ) {
	CoinInventory inventory( { { 100, 5 }, { 50, 5 } } );
	auto result = ChangeCalculator::computeMinimalCoinChange( 150, inventory );

	ASSERT_TRUE( result.has_value() );
	EXPECT_EQ( result->total, 150 );
	EXPECT_EQ( result->coins[ 100 ], 1 );
	EXPECT_EQ( result->coins[ 50 ], 1 );
}

TEST( ChangeCalculatorTests, MinimizesNumberOfCoins ) {
	CoinInventory inventory( { { 100, 5 }, { 50, 5 }, { 20, 5 }, { 10, 5 } } );
	auto result = ChangeCalculator::computeMinimalCoinChange( 200, inventory );

	ASSERT_TRUE( result.has_value() );
	EXPECT_EQ( result->coins[ 100 ], 2 );
	EXPECT_EQ( result->coins.size(), 1 );
}

TEST( ChangeCalculatorTests, RespectsLimitedCoinCounts ) {
	CoinInventory inventory( { { 100, 1 }, { 50, 10 } } );
	auto result = ChangeCalculator::computeMinimalCoinChange( 200, inventory );

	ASSERT_TRUE( result.has_value() );
	EXPECT_EQ( result->coins[ 100 ], 1 );
	EXPECT_EQ( result->coins[ 50 ], 2 );
}

TEST( ChangeCalculatorTests, ReturnsNulloptWhenChangeCannotBeMade ) {
	CoinInventory inventory( { { 200, 10 } } );
	auto result = ChangeCalculator::computeMinimalCoinChange( 220, inventory );

	EXPECT_FALSE( result.has_value() );
}

TEST( ChangeCalculatorTests, ReconstructionDoesNotExceedAvailableCoinCounts ) {
	CoinInventory inventory( { { 10, 2 }, { 4, 2 }, { 3, 3 }, { 1, 1 } } );
	auto result = ChangeCalculator::computeMinimalCoinChange( 12, inventory );

	ASSERT_TRUE( result.has_value() );
	EXPECT_EQ( result->total, 12 );

	Money reconstructed_total    = 0;
	std::size_t total_coin_count = 0;
	for ( const auto& [ denomination, used_count ] : result->coins ) {
		EXPECT_LE( static_cast< std::size_t >( used_count ), inventory.count( denomination ) );
		reconstructed_total += denomination * used_count;
		total_coin_count += static_cast< std::size_t >( used_count );
	}

	EXPECT_EQ( reconstructed_total, 12 );
	EXPECT_EQ( total_coin_count, 4U );
	EXPECT_EQ( result->coins.at( 4 ), 2 );
	EXPECT_EQ( result->coins.at( 3 ), 1 );
	EXPECT_EQ( result->coins.at( 1 ), 1 );
}

}  // namespace task1
