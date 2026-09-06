#pragma once

// SW::LiveLink — the loopback HTTP/WebSocket server (docs/adr/0002).
// Phase 2 of docs/build-checklist.md: a barebones handshake only (version
// negotiation, no scene-editing traffic yet). The heartbeat + live-edit undo
// list (docs/adr/0007) land alongside v1.1's actual command traffic.

#include "protocol/envelope.h"

namespace SW::LiveLink {

// Starts the loopback WebSocket server on plugin load, per adr/0002/0001.
void Init();

// Pure decision logic for the handshake: given a request envelope, decides
// whether to accept it and what to reply. No I/O, no SKSE/RE dependency —
// kept separate from Init()'s networking/thread-marshaling glue so it's
// directly unit-testable (docs/adr/0010, Tier 1).
Protocol::Envelope BuildHelloAck(const Protocol::Envelope& request);

// A rejected HelloAck with the given reason — shared by BuildHelloAck's own
// rejection paths and by livelink_server.cpp's malformed-input/timeout paths.
Protocol::Envelope RejectHello(std::string reason);

}  // namespace SW::LiveLink
