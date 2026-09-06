#include <catch2/catch.hpp>
#include "upf/application/scenario_runner.hpp"

TEST_CASE("built-in uplink and downlink scenarios reach local sinks") {
  upf::ScenarioRunner runner;
  upf::RunResult uplink = runner.run("uplink-forward");
  REQUIRE(uplink.decisions.size() == 1);
  REQUIRE(uplink.decisions[0].terminalResult == upf::FORWARDED_TO_N6);
  REQUIRE(uplink.n6Sink.size() == 1);
  upf::RunResult downlink = runner.run("downlink-forward");
  REQUIRE(downlink.decisions[0].terminalResult == upf::FORWARDED_TO_N3);
  REQUIRE(downlink.n3Sink[0].tunnel.hasValue());
}
