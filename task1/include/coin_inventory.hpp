#pragma once

#include "models.hpp"

#include <array>

namespace task1 {

inline constexpr std::array< Money, 9 > supportedCoinDenominations{ 1, 2, 5, 10, 20, 50, 100, 200, 500 };

[[nodiscard]] bool isSupportedCoinDenomination( Money denomination ) noexcept;

class CoinInventory {
public:
	CoinInventory() = default;

	explicit CoinInventory( std::map< Money, int, std::greater<> > coins );

	[[nodiscard]] const std::map< Money, int, std::greater<> >& getCoins() const;

	void addCoin( Money denomination, int count = 1 );
	void addCoins( const CoinInventory& other );

	bool removeCoin( Money denomination, int count = 1 );
	bool removeCoins( const std::map< Money, int, std::greater<> >& coins );

	[[nodiscard]] std::size_t count( Money denomination ) const;
	[[nodiscard]] Money total() const;

private:
	std::map< Money, int, std::greater<> > coins_;
};

}  // namespace task1