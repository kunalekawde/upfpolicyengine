#include <catch2/catch.hpp>
#include "upf/application/scenario_runner.hpp"

TEST_CASE("human and JSON output derive from the same result") {
  const upf::RunResult result = upf::ScenarioRunner().run("uplink-forward");
  REQUIRE(upf::renderHuman(result).find("PDR: 10") != std::string::npos);
  REQUIRE(upf::renderJson(result).find("\"selectedPdrId\":10") != std::string::npos);
}
