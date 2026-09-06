#include "core/core.h"

// The real engine-touching implementation of core.h's link-seam accessors.
// Only ever linked into the actual SKSE plugin target — the test target
// links tests/fakes/core_fakes.cpp instead (docs/adr/0010).

namespace SW::Core {
    namespace {
        // SKSE::MessagingInterface::RegisterListener only accepts a plain
        // function pointer (no captures), so the caller's callback is stashed
        // here for this free function to invoke.
        std::function<void()> g_dataLoadedCallback;

        void OnSKSEMessage(SKSE::MessagingInterface::Message* message) {
            if (message->type == SKSE::MessagingInterface::kDataLoaded && g_dataLoadedCallback) {
                g_dataLoadedCallback();
            }
        }
    }

    void RegisterDataLoadedListener(std::function<void()> callback) {
        g_dataLoadedCallback = std::move(callback);
        SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
    }

    void LogToConsole(std::string_view message) {
        // The console singleton doesn't exist yet if this is called during
        // SKSEPluginLoad itself (e.g. src/livelink's Init(), which per
        // adr/0002 must start on plugin load, not deferred to kDataLoaded
        // like this module's own callback) — calling Print() on it then
        // would be a null-pointer dereference, not a thrown exception, so
        // it must be guarded rather than left to "just work" like the
        // kDataLoaded-deferred call site this function was originally
        // written for.
        if (auto* console = RE::ConsoleLog::GetSingleton()) {
            console->Print(std::string(message).c_str());
        }
    }
}
