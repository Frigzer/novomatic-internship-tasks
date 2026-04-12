#include "change_calculator.hpp"
#include <limits>
#include <vector>

namespace task1 {

namespace {

struct DpState {
    bool reachable{false};
    int coin_count{std::numeric_limits<int>::max()};
    Money used_coin{0};
};

} // namespace

std::optional<ChangeResult> ChangeCalculator::computeMinimalCoinChange(
    Money amount,
    const CoinInventory& inventory) {

    if (amount < 0) return std::nullopt;
    if (amount == 0) return ChangeResult{0, {}};

    std::vector<DpState> dp(amount + 1);
    dp[0].reachable = true;
    dp[0].coin_count = 0;

    for (const auto& [denomination, available_count] : inventory.getCoins()) {
        for (int i = 0; i < available_count; ++i) { 
            for (Money j = amount; j >= denomination; --j) {
                if (dp[j - denomination].reachable) {
                    int new_count = dp[j - denomination].coin_count + 1;
                    if (!dp[j].reachable || new_count < dp[j].coin_count) {
                        dp[j].reachable = true;
                        dp[j].coin_count = new_count;
                        dp[j].used_coin = denomination;
                    }
                }
            }
        }
    }

    if (!dp[amount].reachable) return std::nullopt;

    ChangeResult result;
    result.total = amount;
    Money current = amount;
    while (current > 0) {
        Money coin = dp[current].used_coin;
        result.coins[coin]++;
        current -= coin;
    }

    return result;
}

} // namespace task1