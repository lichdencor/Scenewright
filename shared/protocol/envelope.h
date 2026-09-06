#pragma once

// SW::Protocol — the versioned live-link wire envelope (docs/adr/0006).
// Single source of truth included by both src/livelink and tool/net, so the
// two independently-rebuilt binaries can never silently drift apart. Not yet
// defined — lands in docs/build-checklist.md Phase 2 alongside the barebones
// live-link handshake.

namespace SW::Protocol {}
