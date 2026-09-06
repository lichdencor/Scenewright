#include "fakes/core_fakes.h"

#include "core/core.h"

// Fake implementation of core.h's link-seam accessors (docs/adr/0010, Tier
// 2) — no SKSE/RE dependency, so tests/core.cpp's real Init() logic can be
// exercised without CommonLibSSE-NG.

namespace SW::Core::Fakes {
    std::function<void()> g_registeredCallback;
    std::string g_lastLoggedMessage;

    void Reset() {
        g_registeredCallback = nullptr;
        g_lastLoggedMessage.clear();
    }
}

namespace SW::Core {
    void RegisterDataLoadedListener(std::function<void()> callback) {
        Fakes::g_registeredCallback = std::move(callback);
    }

    void LogToConsole(std::string_view message) {
        Fakes::g_lastLoggedMessage = message;
    }
}
