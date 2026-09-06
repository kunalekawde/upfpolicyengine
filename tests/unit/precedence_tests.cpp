#include <catch2/catch.hpp>
#include "tests/support/test_builders.hpp"
#include "upf/application/packet_pipeline.hpp"

TEST_CASE("lowest numeric precedence wins") {
  upf::RuleSet rules = upf_test::basicRules();
  upf::Pdr broad = rules.pdrs[0]; broad.id = upf::PdrId(11); broad.precedence = 200;
  rules.pdrs.push_back(broad);
  upf::PacketPipeline pipeline; REQUIRE(pipeline.install(rules).ok());
  REQUIRE(pipeline.process(upf_test::uplinkPacket(), "r", "p").selectedPdrId == 10);
}

TEST_CASE("equal precedence overlapping PDRs are rejected") {
  upf::RuleSet rules = upf_test::basicRules();
  upf::Pdr duplicate = rules.pdrs[0]; duplicate.id = upf::PdrId(11);
  rules.pdrs.push_back(duplicate);
  REQUIRE_FALSE(upf::validateRuleSet(rules, upf::CapacityLimits()).ok());
}
