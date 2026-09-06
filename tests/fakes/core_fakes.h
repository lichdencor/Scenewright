#pragma once

#include <functional>
#include <string>

// Test-visible state for the SW::Core link-seam fake (core_fakes.cpp) —
// lets a test both trigger the registered callback and assert on what got
// logged, without linking CommonLibSSE-NG (docs/adr/0010).

namespace SW::Core::Fakes {
    extern std::function<void()> g_registeredCallback;
    extern std::string g_lastLoggedMessage;

    // Resets both, so tests don't leak state into each other.
    void Reset();
}
