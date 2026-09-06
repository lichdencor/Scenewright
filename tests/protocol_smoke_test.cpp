#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

// Placeholder Tier 1 smoke test (docs/adr/0010-test-suite-strategy.md) —
// proves the doctest harness compiles and runs natively on Linux before any
// real shared/protocol code exists. Replace with real envelope-parsing tests
// once docs/build-checklist.md Phase 2 lands shared/protocol/envelope.h.
TEST_CASE("doctest harness runs") {
    CHECK(1 + 1 == 2);
}
