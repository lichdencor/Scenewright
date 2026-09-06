#include "livelink/livelink.h"

// Engine-agnostic handshake decision logic only — no SKSE/RE, no
// ixwebsocket. Compiled into both the plugin and the test target, same
// split as src/core/core.cpp vs core_engine.cpp (docs/adr/0010, Tier 1/2).
// The actual server (networking + SKSE::GetTaskInterface() marshaling)
// lives in livelink_server.cpp, plugin-only.

namespace SW::LiveLink {

Protocol::Envelope BuildHelloAck(const Protocol::Envelope& request) {
    if (request.type != Protocol::kTypeHello) {
        return RejectHello("expected " + std::string(Protocol::kTypeHello) + ", got " + request.type);
    }
    if (request.version != Protocol::kProtocolVersion) {
        return RejectHello("unsupported protocol version " + std::to_string(request.version));
    }

    return Protocol::Envelope{
        .version = Protocol::kProtocolVersion,
        .type = Protocol::kTypeHelloAck,
        .payload = Protocol::HelloAckPayload{.accepted = true},
    };
}

Protocol::Envelope RejectHello(std::string reason) {
    return Protocol::Envelope{
        .version = Protocol::kProtocolVersion,
        .type = Protocol::kTypeHelloAck,
        .payload = Protocol::HelloAckPayload{.accepted = false, .reason = std::move(reason)},
    };
}

}  // namespace SW::LiveLink
