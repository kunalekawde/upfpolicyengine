#include <catch2/catch.hpp>
#include "tests/support/test_builders.hpp"

TEST_CASE("valid directional rules pass and invalid references fail") {
  upf::RuleSet rules = upf_test::basicRules();
  REQUIRE(upf::validateRuleSet(rules, upf::CapacityLimits()).ok());
  rules.pdrs[0].farId = upf::FarId(999);
  REQUIRE_FALSE(upf::validateRuleSet(rules, upf::CapacityLimits()).ok());
}

TEST_CASE("N3 and N6 tunnel requirements fail closed") {
  upf::RuleSet rules = upf_test::basicRules();
  rules.pdrs[0].teid.reset();
  REQUIRE_FALSE(upf::validateRuleSet(rules, upf::CapacityLimits()).ok());
}
