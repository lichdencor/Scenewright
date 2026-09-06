// SKSEPluginLoad wires modules together only — no logic of its own, per
// docs/adr/0009-code-organization-pattern.md.

#include "core/core.h"

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SW::Core::Init();
    return true;
}
