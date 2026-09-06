#include <doctest/doctest.h>

#include "protocol/envelope.h"

using namespace SW::Protocol;

TEST_CASE("Envelope round-trips through serialize/parse") {
    Envelope original{
        .version = kProtocolVersion,
        .type = kTypeHello,
        .payload = HelloPayload{.protocolVersion = kProtocolVersion, .clientName = "test-client"},
    };

    auto parsed = ParseEnvelope(SerializeEnvelope(original));

    REQUIRE(parsed.has_value());
    CHECK(parsed->version == original.version);
    CHECK(parsed->type == original.type);
    CHECK(parsed->payload.get<HelloPayload>().clientName == "test-client");
}

TEST_CASE("ParseEnvelope rejects malformed JSON") {
    CHECK_FALSE(ParseEnvelope("not json").has_value());
    CHECK_FALSE(ParseEnvelope("{}").has_value());
    CHECK_FALSE(ParseEnvelope(R"({"version": 1, "type": "Hello"})").has_value());
}

TEST_CASE("HelloAckPayload defaults reason to empty when absent") {
    auto ack = nlohmann::json::parse(R"({"accepted": true})").get<HelloAckPayload>();
    CHECK(ack.accepted);
    CHECK(ack.reason.empty());
}
