#include <catch2/catch.hpp>
#include "tests/support/test_builders.hpp"
#include "upf/application/packet_pipeline.hpp"

TEST_CASE("uplink candidate matches all configured fields") {
  upf::PacketPipeline pipeline;
  REQUIRE(pipeline.install(upf_test::basicRules()).ok());
  const upf::PacketDecision decision = pipeline.process(upf_test::uplinkPacket(), "run", "test");
  REQUIRE(decision.selectedPdrId == 10);
  REQUIRE(decision.candidates.size() == 1);
  REQUIRE(decision.candidates[0].matched);
}

TEST_CASE("no match has no policy side effects") {
  upf::Packet packet = upf_test::uplinkPacket();
  packet.tunnel = upf::Optional<upf::TunnelMetadata>(upf::TunnelMetadata(999, 9));
  upf::PacketPipeline pipeline;
  REQUIRE(pipeline.install(upf_test::basicRules()).ok());
  REQUIRE(pipeline.process(packet, "run", "test").terminalResult == upf::NO_MATCH);
}
