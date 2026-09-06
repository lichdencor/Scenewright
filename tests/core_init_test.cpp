#include <doctest/doctest.h>

#include "core/core.h"
#include "fakes/core_fakes.h"

// Proves the link seam (docs/adr/0010, Tier 2): SW::Core::Init()'s real logic
// runs against the fake accessors, with zero CommonLibSSE-NG dependency.
TEST_CASE("Init registers a data-loaded listener that logs the load message") {
    SW::Core::Fakes::Reset();

    SW::Core::Init();

    REQUIRE(SW::Core::Fakes::g_registeredCallback != nullptr);
    SW::Core::Fakes::g_registeredCallback();
    CHECK(SW::Core::Fakes::g_lastLoggedMessage == "Scenewright: plugin loaded.");
}
