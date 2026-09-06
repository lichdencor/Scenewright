#include "livelink/livelink.h"

// The actual networking + thread-marshaling glue — SKSE/RE- and
// ixwebsocket-touching, plugin-only (docs/adr/0010, Tier 2: not covered by
// Tier 1 tests, only by the Phase 2 manual connect/disconnect verification
// in docs/build-checklist.md). BuildHelloAck/RejectHello (livelink.cpp) hold
// all the actually-testable decision logic.

#include <chrono>
#include <future>
#include <optional>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocketServer.h>

#include "config/config.h"
#include "core/core.h"

namespace SW::LiveLink {

namespace {
    // Arbitrary loopback-only port, not yet user-configurable (adr/0002).
    constexpr int kPort = 30201;
    constexpr int kMainThreadResponseTimeoutSeconds = 5;

    // Marshals the actual handshake decision onto the game's main thread
    // (adr/0003) and blocks this connection's own worker thread — never the
    // main thread — until it's ready. Returns std::nullopt if the main
    // thread doesn't respond within the timeout.
    std::optional<Protocol::Envelope> WaitForHelloAck(Protocol::Envelope request) {
        std::promise<Protocol::Envelope> responsePromise;
        auto responseFuture = responsePromise.get_future();

        SKSE::GetTaskInterface()->AddTask([request = std::move(request), &responsePromise] {
            responsePromise.set_value(BuildHelloAck(request));
        });

        if (responseFuture.wait_for(std::chrono::seconds(kMainThreadResponseTimeoutSeconds)) !=
            std::future_status::ready) {
            return std::nullopt;
        }
        return responseFuture.get();
    }

    void LogHandshakeResult(const Protocol::Envelope& response) {
        bool accepted = response.payload.get<Protocol::HelloAckPayload>().accepted;
        Core::LogToConsole(std::string("Scenewright: live-link handshake ") +
                            (accepted ? "accepted" : "rejected"));
    }

    // Runs on this connection's own dedicated worker thread (IXWebSocket
    // gives every accepted connection one) — never the game's main thread,
    // and never any other connection's thread either. webSocket is only
    // ever used here, synchronously, within the lifetime IXWebSocket
    // actually guarantees it for — it is deliberately never captured for
    // later/async use (see WaitForHelloAck).
    void HandleMessage(std::shared_ptr<ix::ConnectionState>, ix::WebSocket& webSocket,
                        const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            Core::LogToConsole("Scenewright: live-link client connected");
            return;
        }
        if (msg->type == ix::WebSocketMessageType::Close) {
            Core::LogToConsole("Scenewright: live-link client disconnected");
            return;
        }
        if (msg->type != ix::WebSocketMessageType::Message) {
            return;
        }

        Protocol::Envelope response;
        if (auto request = Protocol::ParseEnvelope(msg->str)) {
            response = WaitForHelloAck(*request).value_or(RejectHello("main thread did not respond in time"));
        } else {
            // adr/0007: malformed input from the untrusted peer never reaches
            // BuildHelloAck/AddTask at all.
            response = RejectHello("malformed envelope");
        }

        webSocket.send(Protocol::SerializeEnvelope(response));
        LogHandshakeResult(response);
    }
}  // namespace

void Init() {
    // Dev-only tooling: a normal player's install has no reason to ever
    // bind this port. See docs/adr/0001's authoring-tool/actual-playback
    // split — the live-link server only exists to prove the wire for
    // scene authors, never for someone just experiencing a finished scene.
    if (!Config::IsDevModeEnabled()) {
        Core::LogToConsole("Scenewright: live-link server disabled (set EnableLiveLink=1 under [Debug] in Scenewright.ini to enable)");
        return;
    }

    ix::initNetSystem();

    static ix::WebSocketServer server(kPort, "127.0.0.1");
    server.setOnClientMessageCallback(HandleMessage);

    auto result = server.listen();
    if (!result.first) {
        Core::LogToConsole("Scenewright: live-link server failed to start: " + result.second);
        return;
    }

    server.start();
    Core::LogToConsole("Scenewright: live-link server listening on 127.0.0.1:" + std::to_string(kPort));
}

}  // namespace SW::LiveLink
