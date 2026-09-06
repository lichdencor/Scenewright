#include <doctest/doctest.h>

#include "livelink/livelink.h"

using namespace SW::LiveLink;
using namespace SW::Protocol;

TEST_CASE("BuildHelloAck accepts a matching Hello") {
    Envelope request{.version = kProtocolVersion, .type = kTypeHello,
                      .payload = HelloPayload{.protocolVersion = kProtocolVersion, .clientName = "tool"}};

    auto response = BuildHelloAck(request);

    CHECK(response.type == kTypeHelloAck);
    CHECK(response.payload.get<HelloAckPayload>().accepted);
}

TEST_CASE("BuildHelloAck rejects a version mismatch") {
    Envelope request{.version = kProtocolVersion + 1, .type = kTypeHello, .payload = nlohmann::json::object()};

    auto response = BuildHelloAck(request);
    auto ack = response.payload.get<HelloAckPayload>();

    CHECK_FALSE(ack.accepted);
    CHECK_FALSE(ack.reason.empty());
}

TEST_CASE("BuildHelloAck rejects a non-Hello message type") {
    Envelope request{.version = kProtocolVersion, .type = "SomethingElse", .payload = nlohmann::json::object()};

    auto response = BuildHelloAck(request);

    CHECK_FALSE(response.payload.get<HelloAckPayload>().accepted);
}
