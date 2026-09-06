#include <catch2/catch.hpp>
#include "upf/application/scenario_runner.hpp"

TEST_CASE("adapter failure scenario exposes divergence and recovery facts") {
  const upf::RunResult result = upf::ScenarioRunner().run("adapter-failure");
  REQUIRE(result.intendedVersion == 3);
  REQUIRE(result.activeVersion == 3);
  REQUIRE(result.appliedVersion == 3);
  REQUIRE(result.convergence == upf::CONVERGED);
}
