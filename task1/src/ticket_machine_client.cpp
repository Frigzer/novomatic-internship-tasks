#include "ticket_machine_client.hpp"

namespace task1 {

TicketMachineClient::TicketMachineClient(TicketServer& server)
    : server_(server) {}

std::vector<TicketAvailability> TicketMachineClient::showAvailableTickets() {
    return server_.getAvailableTickets();
}

std::optional<ReservationResult> TicketMachineClient::selectTicket(std::string_view ticket_type) {
    return server_.reserveTicket(std::string(ticket_type));
}

bool TicketMachineClient::cancel(ReservationId reservation_id) {
    return server_.cancelReservation(reservation_id);
}

std::variant<PurchaseSuccess, PurchaseFailure> TicketMachineClient::buy(
    ReservationId reservation_id,
    const CustomerData& customer,
    const CoinInventory& inserted_coins) {
    return server_.finalizePurchase(reservation_id, customer, inserted_coins);
}

} // namespace task1