#include "core/core.h"

// Engine-agnostic glue only — no SKSE/RE types here, by design (see core.h).
// This is what makes Init() testable via the link seam in docs/adr/0010.

namespace SW::Core {
    void Init() {
        RegisterDataLoadedListener([] { LogToConsole("Scenewright: plugin loaded."); });
    }
}
