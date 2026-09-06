#include "protocol/envelope.h"

namespace SW::Protocol {

void to_json(nlohmann::json& j, const Envelope& envelope) {
    j = nlohmann::json{
        {"version", envelope.version},
        {"type", envelope.type},
        {"payload", envelope.payload},
    };
}

void from_json(const nlohmann::json& j, Envelope& envelope) {
    envelope.version = j.at("version").get<int>();
    envelope.type = j.at("type").get<std::string>();
    envelope.payload = j.at("payload");
}

std::optional<Envelope> ParseEnvelope(const std::string& raw) {
    try {
        return nlohmann::json::parse(raw).get<Envelope>();
    } catch (const nlohmann::json::exception&) {
        return std::nullopt;
    }
}

std::string SerializeEnvelope(const Envelope& envelope) {
    return nlohmann::json(envelope).dump();
}

void to_json(nlohmann::json& j, const HelloPayload& hello) {
    j = nlohmann::json{
        {"protocolVersion", hello.protocolVersion},
        {"clientName", hello.clientName},
    };
}

void from_json(const nlohmann::json& j, HelloPayload& hello) {
    hello.protocolVersion = j.at("protocolVersion").get<int>();
    hello.clientName = j.at("clientName").get<std::string>();
}

void to_json(nlohmann::json& j, const HelloAckPayload& ack) {
    j = nlohmann::json{
        {"accepted", ack.accepted},
        {"reason", ack.reason},
    };
}

void from_json(const nlohmann::json& j, HelloAckPayload& ack) {
    ack.accepted = j.at("accepted").get<bool>();
    ack.reason = j.value("reason", std::string{});
}

}  // namespace SW::Protocol
