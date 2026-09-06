#pragma once

// SW::LiveLink — the loopback HTTP/WebSocket server (docs/adr/0002), envelope
// parsing/dispatch marshaled onto the game's main thread via
// SKSE::GetTaskInterface()->AddTask() (docs/adr/0003), and the heartbeat +
// live-edit undo list (docs/adr/0007). Not yet implemented — the barebones
// handshake (version/capability negotiation only, no scene-editing traffic)
// is docs/build-checklist.md Phase 2.

namespace SW::LiveLink {}
