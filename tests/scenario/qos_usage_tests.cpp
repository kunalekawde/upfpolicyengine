#include <catch2/catch.hpp>
#include "upf/application/scenario_runner.hpp"
#include "upf/domain/engine.hpp"
#include <limits>

TEST_CASE("zero rate spends initial burst then rate limits") {
  upf::Qer qer; qer.id = upf::QerId(30); qer.gate = upf::OPEN;
  qer.rateBitsPerSecond = 0; qer.burstBytes = 100;
  upf::TokenBucketState state; state.tokensBytes = 100;
  upf::QerEvaluation first = upf::evaluateQer(qer, state, 50, 1);
  REQUIRE(first.decision.allowed);
  upf::QerEvaluation second = upf::evaluateQer(qer, first.next, 60, 2);
  REQUIRE_FALSE(second.decision.allowed);
  REQUIRE(second.decision.outcome == "RATE_LIMITED");
}

TEST_CASE("zero burst and arithmetic overflow fail without token mutation") {
  upf::Qer qer; qer.id = upf::QerId(30); qer.gate = upf::OPEN; qer.burstBytes = 0;
  upf::TokenBucketState state;
  REQUIRE_FALSE(upf::evaluateQer(qer, state, 1, 0).decision.allowed);
  qer.rateBitsPerSecond = std::numeric_limits<std::uint64_t>::max();
  REQUIRE(upf::evaluateQer(qer, state, 1, 2).failure == upf::ARITHMETIC_ERROR);
  REQUIRE(state.tokensBytes == 0);
}

TEST_CASE("URR threshold emits once per generation") {
  upf::Urr urr; urr.id = upf::UrrId(40); urr.volumeThresholdBytes = 100; urr.generation = 7;
  upf::UsageCounterState state;
  upf::UrrEvaluation first = upf::evaluateUrr(urr, state, 100, "s", 1);
  REQUIRE(first.reportGenerated);
  upf::UrrEvaluation second = upf::evaluateUrr(urr, first.next, 100, "s", 1);
  REQUIRE_FALSE(second.reportGenerated);
  REQUIRE(second.next.payloadBytes == 200);
}

TEST_CASE("built-in QoS and threshold scenarios expose outcomes") {
  const upf::RunResult rate = upf::ScenarioRunner().run("qer-rate-limit");
  REQUIRE(rate.decisions.size() == 2);
  REQUIRE(rate.decisions[1].terminalResult == upf::RATE_LIMITED);
  const upf::RunResult threshold = upf::ScenarioRunner().run("urr-threshold");
  REQUIRE(threshold.decisions.back().urr.thresholdReached);
}
