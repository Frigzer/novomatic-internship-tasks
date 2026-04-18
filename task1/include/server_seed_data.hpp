#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <filesystem>
#include <vector>

namespace task1 {

struct ServerSeedData {
    std::vector<Ticket> tickets;
    CoinInventory cashbox;
};

class ServerSeedDataLoader {
public:
    [[nodiscard]] static ServerSeedData loadFromFile(const std::filesystem::path& file_path);
};

}  // namespace task1
