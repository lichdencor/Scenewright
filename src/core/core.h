#pragma once

#include <functional>
#include <string_view>

// SW::Core — logging, SKSE lifecycle messages, and the wrapped access points
// for SKSE/RE singletons. Per docs/adr/0009 this is the only module allowed
// to touch raw SKSE::/RE:: singletons directly; per docs/adr/0010 the two
// accessors below are the link-seam boundary swapped out in test builds.
// Nothing in this header includes SKSE/RE headers, so anything that only
// calls these — like Init() — never needs CommonLibSSE-NG to compile.

namespace SW::Core {
    // One-time plugin setup: registers SKSE lifecycle listeners. Called once
    // from SKSEPluginLoad. Engine-agnostic — only calls the two accessors
    // below, defined in src/core/core.cpp so it can be linked against either
    // the real engine (src/core/core_engine.cpp) or a test fake
    // (tests/fakes/core_fakes.cpp) without recompiling.
    void Init();

    // Link-seam accessors — the only two functions in this codebase allowed
    // to reach into raw SKSE::/RE:: singletons. Everything else calls these,
    // never SKSE::GetMessagingInterface()/RE::ConsoleLog::GetSingleton()
    // directly, so "who touches the engine" is always these two. Real
    // implementation: src/core/core_engine.cpp. Test fake:
    // tests/fakes/core_fakes.cpp.
    void RegisterDataLoadedListener(std::function<void()> callback);
    void LogToConsole(std::string_view message);
}
