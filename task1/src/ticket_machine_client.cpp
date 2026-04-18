#include "ticket_machine_client.hpp"

#include "network_protocol.hpp"

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include <istream>
#include <stdexcept>
#include <string>

namespace task1 {
namespace {
using asio::ip::tcp;
using Json = nlohmann::json;

void ensureOkResponse(const Json& response) {
    if (!response.value("ok", false)) {
        throw std::runtime_error(response.value("message", std::string("Unknown server error")));
    }
}

}  // namespace

TicketMachineClient::TicketMachineClient(TicketServer& server) : backend_(&server) {}

TicketMachineClient::TicketMachineClient(std::string host, std::uint16_t port)
    : backend_(RemoteEndpoint{.host = std::move(host), .port = port}) {}

std::vector<TicketAvailability> TicketMachineClient::showAvailableTickets() {
    if (!isRemote()) {
        return localServer().getAvailableTickets();
    }

    const Json response = sendRemoteRequest(Json{{"action", "list_tickets"}});
    ensureOkResponse(response);
    return protocol::availabilityListFromJson(response.at("tickets"));
}

std::optional<ReservationResult> TicketMachineClient::selectTicket(std::string_view ticket_type) {
    if (!isRemote()) {
        return localServer().reserveTicket(std::string(ticket_type));
    }

    const Json response = sendRemoteRequest(Json{{"action", "reserve_ticket"}, {"ticket_type", std::string(ticket_type)}});
    ensureOkResponse(response);
    if (!response.value("reserved", false)) {
        return std::nullopt;
    }
    return protocol::reservationFromJson(response.at("reservation"));
}

bool TicketMachineClient::cancel(ReservationId reservation_id) {
    if (!isRemote()) {
        return localServer().cancelReservation(reservation_id);
    }

    const Json response = sendRemoteRequest(Json{{"action", "cancel_reservation"}, {"reservation_id", reservation_id}});
    ensureOkResponse(response);
    return response.at("cancelled").get<bool>();
}

std::variant<PurchaseSuccess, PurchaseFailure> TicketMachineClient::buy(
    ReservationId reservation_id, const CustomerData& customer, const CoinInventory& inserted_coins) {
    if (!isRemote()) {
        return localServer().finalizePurchase(reservation_id, customer, inserted_coins);
    }

    const Json response = sendRemoteRequest(
        Json{{"action", "finalize_purchase"},
             {"reservation_id", reservation_id},
             {"customer", protocol::toJson(customer)},
             {"inserted_coins", protocol::toJson(inserted_coins)}});
    ensureOkResponse(response);

    if (response.at("success").get<bool>()) {
        return protocol::purchaseSuccessFromJson(response.at("purchase"));
    }
    return protocol::purchaseFailureFromJson(response.at("purchase"));
}

bool TicketMachineClient::isRemote() const noexcept {
    return std::holds_alternative<RemoteEndpoint>(backend_);
}

TicketServer& TicketMachineClient::localServer() const {
    auto* server = std::get_if<TicketServer*>(&backend_);
    if (server == nullptr || *server == nullptr) {
        throw std::logic_error("Local ticket server backend is not available");
    }
    return **server;
}

const TicketMachineClient::RemoteEndpoint& TicketMachineClient::remoteEndpoint() const {
    const auto* endpoint = std::get_if<RemoteEndpoint>(&backend_);
    if (endpoint == nullptr) {
        throw std::logic_error("Remote ticket server backend is not available");
    }
    return *endpoint;
}

Json TicketMachineClient::sendRemoteRequest(const Json& request) const {
    const auto& endpoint = remoteEndpoint();

    asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::socket socket(io_context);

    asio::error_code error;
    const auto endpoints = resolver.resolve(endpoint.host, std::to_string(endpoint.port), error);
    if (error) {
        throw std::runtime_error("Could not resolve server address: " + error.message());
    }

    asio::connect(socket, endpoints, error);
    if (error) {
        throw std::runtime_error("Could not connect to server: " + error.message());
    }

    const auto payload = request.dump() + "\n";
    asio::write(socket, asio::buffer(payload), error);
    if (error) {
        throw std::runtime_error("Could not send request to server: " + error.message());
    }

    asio::streambuf response_buffer;
    asio::read_until(socket, response_buffer, '\n', error);
    if (error) {
        throw std::runtime_error("Could not read response from server: " + error.message());
    }

    std::istream response_stream(&response_buffer);
    std::string response_line;
    std::getline(response_stream, response_line);
    if (response_line.empty()) {
        throw std::runtime_error("Received an empty response from the server");
    }

    return Json::parse(response_line);
}

}  // namespace task1
