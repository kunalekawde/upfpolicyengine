#include <catch2/catch.hpp>
#include "upf/domain/engine.hpp"

TEST_CASE("URR counts original payload once") {
  upf::Urr urr; urr.id = upf::UrrId(40); urr.volumeThresholdBytes = 10000;
  upf::UsageCounterState state;
  const upf::UrrEvaluation result = upf::evaluateUrr(urr, state, 1200, "s", 3);
  REQUIRE(result.next.packets == 1);
  REQUIRE(result.next.payloadBytes == 1200);
  REQUIRE_FALSE(result.reportGenerated);
}
