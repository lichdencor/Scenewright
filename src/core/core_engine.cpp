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
        RE::ConsoleLog::GetSingleton()->Print(std::string(message).c_str());
    }
}
