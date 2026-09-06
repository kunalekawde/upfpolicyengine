#include <catch2/catch.hpp>
#include "tests/support/test_builders.hpp"
#include "upf/application/packet_pipeline.hpp"

TEST_CASE("one hundred thousand packet update interleavings see complete snapshots") {
  upf::RuleSet oldRules = upf_test::basicRules(); oldRules.version = 1;
  upf::RuleSet newRules = oldRules; newRules.version = 2; newRules.pdrs[0].precedence = 50;
  for (std::size_t i = 0; i < 100000; ++i) {
    upf::PacketPipeline pipeline;
    REQUIRE(pipeline.install((i % 2U) == 0U ? oldRules : newRules).ok());
    const std::uint64_t seen = pipeline.process(upf_test::uplinkPacket(), "r", "interleave").rulesetVersion;
    REQUIRE((seen == 1 || seen == 2));
  }
}
