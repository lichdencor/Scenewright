#include <cstddef>
#include <cstdint>
#include <string>

#include "protocol/envelope.h"

// Tier 3 (docs/adr/0010): exercises the untrusted-peer boundary (adr/0007)
// directly — ParseEnvelope must never throw or crash on any byte sequence a
// hostile/broken peer could send, and re-serializing anything it does
// successfully parse must be equally safe.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    auto envelope = SW::Protocol::ParseEnvelope(std::string(reinterpret_cast<const char*>(data), size));
    if (envelope) {
        (void)SW::Protocol::SerializeEnvelope(*envelope);
    }
    return 0;
}
