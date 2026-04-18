#include "change_calculator.hpp"

#include <limits>
#include <vector>

namespace task1 {

std::optional< ChangeResult > ChangeCalculator::computeMinimalCoinChange( Money amount,
                                                                          const CoinInventory& inventory ) {
	if ( amount < 0 ) return std::nullopt;
	if ( amount == 0 ) return ChangeResult{ .total = 0, .coins = {} };

	struct CoinType {
		Money denomination{ 0 };
		int available_count{ 0 };
	};

	std::vector< CoinType > coin_types;
	coin_types.reserve( inventory.getCoins().size() );
	for ( const auto& [ denomination, available_count ] : inventory.getCoins() ) {
		if ( denomination <= 0 || available_count <= 0 ) {
			continue;
		}
		coin_types.push_back( CoinType{ .denomination = denomination, .available_count = available_count } );
	}

	constexpr int unreachable = std::numeric_limits< int >::max() / 4;
	const std::size_t type_count = coin_types.size();
	const std::size_t target_amount = static_cast< std::size_t >( amount );

	std::vector< std::vector< int > > dp( type_count + 1, std::vector< int >( target_amount + 1, unreachable ) );
	std::vector< std::vector< int > > chosen_counts( type_count + 1,
	                                                std::vector< int >( target_amount + 1, -1 ) );

	dp[ 0 ][ 0 ] = 0;
	chosen_counts[ 0 ][ 0 ] = 0;

	for ( std::size_t type_index = 1; type_index <= type_count; ++type_index ) {
		const CoinType& coin = coin_types[ type_index - 1 ];
		for ( std::size_t current_amount = 0; current_amount <= target_amount; ++current_amount ) {
			for ( int used_count = 0; used_count <= coin.available_count; ++used_count ) {
				const Money used_value = coin.denomination * used_count;
				if ( used_value > static_cast< Money >( current_amount ) ) {
					break;
				}

				const std::size_t previous_amount =
				    current_amount - static_cast< std::size_t >( used_value );
				if ( dp[ type_index - 1 ][ previous_amount ] == unreachable ) {
					continue;
				}

				const int candidate_coin_count = dp[ type_index - 1 ][ previous_amount ] + used_count;
				if ( candidate_coin_count < dp[ type_index ][ current_amount ] ) {
					dp[ type_index ][ current_amount ] = candidate_coin_count;
					chosen_counts[ type_index ][ current_amount ] = used_count;
				}
			}
		}
	}

	if ( dp[ type_count ][ target_amount ] == unreachable ) {
		return std::nullopt;
	}

	ChangeResult result;
	result.total = amount;

	std::size_t current_amount = target_amount;
	for ( std::size_t type_index = type_count; type_index > 0; --type_index ) {
		const CoinType& coin = coin_types[ type_index - 1 ];
		const int used_count = chosen_counts[ type_index ][ current_amount ];
		if ( used_count < 0 ) {
			return std::nullopt;
		}

		if ( used_count > 0 ) {
			result.coins[ coin.denomination ] = used_count;
			current_amount -= static_cast< std::size_t >( coin.denomination * used_count );
		}
	}

	if ( current_amount != 0 ) {
		return std::nullopt;
	}

	return result;
}

}  // namespace task1
