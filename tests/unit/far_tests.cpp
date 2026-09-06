#include <catch2/catch.hpp>
#include "tests/support/test_builders.hpp"
#include "upf/domain/engine.hpp"

TEST_CASE("uplink FAR removes tunnel metadata") {
  upf::Far far = upf_test::basicRules().fars[0];
  const upf::FarEvaluation result = upf::evaluateFar(far, upf_test::uplinkPacket());
  REQUIRE(result.valid);
  REQUIRE(result.forwarded);
  REQUIRE_FALSE(result.packet.tunnel.hasValue());
}
