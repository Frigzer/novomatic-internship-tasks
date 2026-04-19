#include "change_calculator.hpp"

#include <limits>
#include <vector>

namespace task1 {
namespace {

struct CoinType {
	Money denomination{ 0 };
	int available_count{ 0 };
};

constexpr int unreachable = std::numeric_limits< int >::max() / 4;

using DpTable = std::vector< std::vector< int > >;

std::vector< CoinType > collectCoinTypes( const CoinInventory& inventory ) {
	std::vector< CoinType > coin_types;
	coin_types.reserve( inventory.getCoins().size() );

	for ( const auto& [ denomination, available_count ] : inventory.getCoins() ) {
		if ( denomination <= 0 || available_count <= 0 ) {
			continue;
		}

		coin_types.push_back( CoinType{ .denomination = denomination, .available_count = available_count } );
	}

	return coin_types;
}

struct DynamicProgrammingState {
	DpTable dp;
	DpTable chosen_counts;
};

void updateBestChoiceForAmount( DynamicProgrammingState& state, const CoinType& coin, std::size_t type_index,
                                std::size_t current_amount ) {
	for ( int used_count = 0; used_count <= coin.available_count; ++used_count ) {
		const Money used_value = coin.denomination * used_count;
		if ( used_value > static_cast< Money >( current_amount ) ) {
			break;
		}

		const std::size_t previous_amount = current_amount - static_cast< std::size_t >( used_value );
		if ( state.dp[ type_index - 1 ][ previous_amount ] == unreachable ) {
			continue;
		}

		const int candidate_coin_count = state.dp[ type_index - 1 ][ previous_amount ] + used_count;
		if ( candidate_coin_count >= state.dp[ type_index ][ current_amount ] ) {
			continue;
		}

		state.dp[ type_index ][ current_amount ]            = candidate_coin_count;
		state.chosen_counts[ type_index ][ current_amount ] = used_count;
	}
}

DynamicProgrammingState computeDpTables( Money amount, const std::vector< CoinType >& coin_types ) {
	const std::size_t type_count = coin_types.size();
	const auto target_amount     = static_cast< std::size_t >( amount );

	DynamicProgrammingState state{
	    .dp            = DpTable( type_count + 1, std::vector< int >( target_amount + 1, unreachable ) ),
	    .chosen_counts = DpTable( type_count + 1, std::vector< int >( target_amount + 1, -1 ) ) };

	state.dp[ 0 ][ 0 ]            = 0;
	state.chosen_counts[ 0 ][ 0 ] = 0;

	for ( std::size_t type_index = 1; type_index <= type_count; ++type_index ) {
		const CoinType& coin = coin_types[ type_index - 1 ];
		for ( std::size_t current_amount = 0; current_amount <= target_amount; ++current_amount ) {
			updateBestChoiceForAmount( state, coin, type_index, current_amount );
		}
	}

	return state;
}

std::optional< ChangeResult > reconstructResult( Money amount, const std::vector< CoinType >& coin_types,
                                                 const DpTable& chosen_counts ) {
	ChangeResult result;
	result.total = amount;

	auto current_amount = static_cast< std::size_t >( amount );

	for ( std::size_t type_index = coin_types.size(); type_index > 0; --type_index ) {
		const CoinType& coin = coin_types[ type_index - 1 ];
		const int used_count = chosen_counts[ type_index ][ current_amount ];

		if ( used_count < 0 ) {
			return std::nullopt;
		}

		if ( used_count == 0 ) {
			continue;
		}

		result.coins[ coin.denomination ] = used_count;
		current_amount -= static_cast< std::size_t >( coin.denomination * used_count );
	}

	if ( current_amount != 0 ) {
		return std::nullopt;
	}

	return result;
}

}  // namespace

std::optional< ChangeResult > ChangeCalculator::computeMinimalCoinChange( Money amount,
                                                                          const CoinInventory& inventory ) {
	if ( amount < 0 ) {
		return std::nullopt;
	}

	if ( amount == 0 ) {
		return ChangeResult{ .total = 0, .coins = {} };
	}

	const auto coin_types = collectCoinTypes( inventory );
	const auto state      = computeDpTables( amount, coin_types );

	const auto target_amount = static_cast< std::size_t >( amount );
	if ( state.dp[ coin_types.size() ][ target_amount ] == unreachable ) {
		return std::nullopt;
	}

	return reconstructResult( amount, coin_types, state.chosen_counts );
}

}  // namespace task1