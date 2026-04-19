#include "coin_inventory.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace task1 {

namespace {

void validateCoinEntry( Money denomination, int count ) {
	if ( denomination <= 0 ) {
		throw std::invalid_argument( "Coin denomination must be positive" );
	}
	if ( !isSupportedCoinDenomination( denomination ) ) {
		throw std::invalid_argument( "Unsupported coin denomination" );
	}
	if ( count < 0 ) {
		throw std::invalid_argument( "Coin count cannot be negative" );
	}
}

}  // namespace

bool isSupportedCoinDenomination( Money denomination ) noexcept {
	return std::ranges::contains( supportedCoinDenominations, denomination );
}

CoinInventory::CoinInventory( std::map< Money, int, std::greater<> > coins ) : coins_( std::move( coins ) ) {
	for ( const auto& [ denomination, count ] : coins_ ) {
		validateCoinEntry( denomination, count );
	}
}

const std::map< Money, int, std::greater<> >& CoinInventory::getCoins() const {
	return coins_;
}

void CoinInventory::addCoin( Money denomination, int count ) {
	validateCoinEntry( denomination, count );
	if ( count == 0 ) {
		return;
	}

	coins_[ denomination ] += count;
}

void CoinInventory::addCoins( const CoinInventory& other ) {
	for ( const auto& [ denomination, count ] : other.getCoins() ) {
		addCoin( denomination, count );
	}
}

bool CoinInventory::removeCoin( Money denomination, int count ) {
	if ( denomination <= 0 || !isSupportedCoinDenomination( denomination ) || count <= 0 ) {
		return false;
	}

	auto it = coins_.find( denomination );
	if ( it == coins_.end() || it->second < count ) {
		return false;
	}

	it->second -= count;
	if ( it->second == 0 ) {
		coins_.erase( it );
	}

	return true;
}

bool CoinInventory::removeCoins( const std::map< Money, int, std::greater<> >& coins ) {
	for ( const auto& [ denomination, count ] : coins ) {
		if ( denomination <= 0 || !isSupportedCoinDenomination( denomination ) || count < 0 ) {
			return false;
		}

		auto it = coins_.find( denomination );
		if ( it == coins_.end() || it->second < count ) {
			return false;
		}
	}

	for ( const auto& [ denomination, count ] : coins ) {
		removeCoin( denomination, count );
	}

	return true;
}

std::size_t CoinInventory::count( Money denomination ) const {
	auto it = coins_.find( denomination );
	if ( it == coins_.end() ) {
		return 0;
	}

	return static_cast< std::size_t >( it->second );
}

Money CoinInventory::total() const {
	Money sum = 0;
	for ( const auto& [ denomination, count ] : coins_ ) {
		sum += denomination * count;
	}
	return sum;
}

}  // namespace task1
