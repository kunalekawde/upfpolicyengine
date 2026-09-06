#include <catch2/catch.hpp>
#include "tests/support/test_builders.hpp"
#include "upf/application/rule_service.hpp"

TEST_CASE("invalid update leaves complete prior snapshot active") {
  upf::RuleService service;
  upf::RuleSet first = upf_test::basicRules(); first.version = 1;
  REQUIRE(service.update("u1", first).ok());
  upf::RuleSet invalid = first; invalid.version = 2; invalid.pdrs[0].farId = upf::FarId(999);
  REQUIRE_FALSE(service.update("u2", invalid).ok());
  REQUIRE(service.active()->version == 1);
}

TEST_CASE("versions increase monotonically") {
  upf::RuleService service;
  upf::RuleSet first = upf_test::basicRules(); first.version = 1;
  REQUIRE(service.update("u1", first).ok());
  REQUIRE_FALSE(service.update("u2", first).ok());
}

TEST_CASE("runtime state migrates only while rule generations are unchanged") {
  upf::RuleService service;
  upf::RuleSet first = upf_test::basicRules();
  first.version = 1;
  REQUIRE(service.update("u1", first).ok());
  service.runtime().qers[30].tokensBytes = 123;
  service.runtime().urrs[40].payloadBytes = 456;

  upf::RuleSet unchanged = first;
  unchanged.version = 2;
  REQUIRE(service.update("u2", unchanged).ok());
  REQUIRE(service.runtime().qers.find(30)->second.tokensBytes == 123);
  REQUIRE(service.runtime().urrs.find(40)->second.payloadBytes == 456);

  upf::RuleSet regenerated = unchanged;
  regenerated.version = 3;
  regenerated.qers[0].generation = 2;
  regenerated.urrs[0].generation = 2;
  REQUIRE(service.update("u3", regenerated).ok());
  REQUIRE(service.runtime().qers.find(30)->second.tokensBytes == regenerated.qers[0].burstBytes);
  REQUIRE(service.runtime().urrs.find(40)->second.payloadBytes == 0);
}
