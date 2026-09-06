#include <catch2/catch.hpp>
#include "upf/application/scenario_runner.hpp"

TEST_CASE("required built-in scenario names are stable") {
  const std::vector<std::string> names = upf::ScenarioRunner::scenarioNames();
  REQUIRE(names.size() >= 6);
  REQUIRE(names[0] == "uplink-forward");
  REQUIRE(names[1] == "downlink-forward");
}
