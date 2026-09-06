#pragma once

// SW::Config — reads Scenewright.ini. Engine-agnostic (no SKSE/RE types),
// so it's directly unit-testable (docs/adr/0010, Tier 1) despite living
// under src/ rather than shared/ — it's plugin-specific, not something the
// external tool (Phase 4) also needs to parse.

#include <string_view>

namespace SW::Config {

inline constexpr const char* kDefaultIniPath = "Data/SKSE/Plugins/Scenewright.ini";

// Gates the live-link server (src/livelink) from ever starting for a normal
// player — a finished mod install has no reason to carry this ini file, or
// this key, at all. Defaults to false (off) when the file or key is
// missing. iniPath is overridable so this is testable without touching the
// real game's Data folder.
bool IsDevModeEnabled(std::string_view iniPath = kDefaultIniPath);

}  // namespace SW::Config
