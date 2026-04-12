#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <optional>

namespace task1 {

class ChangeCalculator {
public:
    static std::optional<ChangeResult> computeMinimalCoinChange(
        Money amount,
        const CoinInventory& inventory);
};

} // namespace task1