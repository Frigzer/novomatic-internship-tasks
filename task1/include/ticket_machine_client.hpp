#pragma once

#include "ticket_server.hpp"

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace task1 {

class TicketMachineClient {
public:
    explicit TicketMachineClient(TicketServer& server);
    TicketMachineClient(std::string host, std::uint16_t port);

    std::vector<TicketAvailability> showAvailableTickets();
    std::optional<ReservationResult> selectTicket(std::string_view ticket_type);
    bool cancel(ReservationId reservation_id);
    std::variant<PurchaseSuccess, PurchaseFailure> buy(
        ReservationId reservation_id, const CustomerData& customer, const CoinInventory& inserted_coins);
    void ping();

private:
    struct RemoteEndpoint {
        std::string host;
        std::uint16_t port{0};
    };

    [[nodiscard]] bool isRemote() const noexcept;
    [[nodiscard]] TicketServer& localServer() const;
    [[nodiscard]] const RemoteEndpoint& remoteEndpoint() const;
    nlohmann::json sendRemoteRequest(const nlohmann::json& request) const;

    std::variant<TicketServer*, RemoteEndpoint> backend_;
};

}  // namespace task1
