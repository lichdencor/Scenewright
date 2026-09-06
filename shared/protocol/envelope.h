#pragma once

// Wire protocol envelope, per docs/adr/0006-wire-protocol.md: a hand-rolled
// versioned JSON envelope, {"version": N, "type": "...", "payload": {...}}.
// Shared between the SKSE plugin (src/livelink) and the external tool
// (Phase 4 of docs/build-checklist.md) — this header must never depend on
// SKSE::/RE:: types.

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace SW::Protocol {

// Bumped when the envelope shape itself changes, not per message type.
inline constexpr int kProtocolVersion = 1;

inline constexpr const char* kTypeHello = "Hello";
inline constexpr const char* kTypeHelloAck = "HelloAck";

struct Envelope {
    int version = kProtocolVersion;
    std::string type;
    nlohmann::json payload;
};

void to_json(nlohmann::json& j, const Envelope& envelope);
void from_json(const nlohmann::json& j, Envelope& envelope);

// Parses raw wire text into an Envelope. Returns std::nullopt on malformed
// JSON or a missing required field — never throws, per adr/0007's rule that
// a malformed message from the untrusted peer must be rejected before it
// reaches anything else.
std::optional<Envelope> ParseEnvelope(const std::string& raw);

std::string SerializeEnvelope(const Envelope& envelope);

struct HelloPayload {
    int protocolVersion = kProtocolVersion;
    std::string clientName;
};

void to_json(nlohmann::json& j, const HelloPayload& hello);
void from_json(const nlohmann::json& j, HelloPayload& hello);

struct HelloAckPayload {
    bool accepted = false;
    std::string reason;  // populated only when accepted == false
};

void to_json(nlohmann::json& j, const HelloAckPayload& ack);
void from_json(const nlohmann::json& j, HelloAckPayload& ack);

}  // namespace SW::Protocol
