#include <catch2/catch.hpp>
#include "upf/domain/engine.hpp"

TEST_CASE("open QER starts full and charges payload") {
  upf::Qer qer;
  qer.id = upf::QerId(30); qer.gate = upf::OPEN;
  qer.rateBitsPerSecond = 10000000; qer.burstBytes = 20000;
  upf::TokenBucketState state; state.tokensBytes = qer.burstBytes;
  const upf::QerEvaluation result = upf::evaluateQer(qer, state, 1200, 0);
  REQUIRE(result.decision.allowed);
  REQUIRE(result.next.tokensBytes == 18800);
}

TEST_CASE("closed QER does not mutate tokens") {
  upf::Qer qer; qer.id = upf::QerId(30); qer.gate = upf::CLOSED; qer.burstBytes = 100;
  upf::TokenBucketState state; state.tokensBytes = 100;
  const upf::QerEvaluation result = upf::evaluateQer(qer, state, 10, 1000);
  REQUIRE_FALSE(result.decision.allowed);
  REQUIRE(result.next.tokensBytes == 100);
}
