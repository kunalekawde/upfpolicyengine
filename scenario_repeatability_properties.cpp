#include <catch2/catch.hpp>
#include "upf/application/scenario_runner.hpp"

TEST_CASE("scenario_repeatability_1000") {
  const std::vector<std::string> names = upf::ScenarioRunner::scenarioNames();
  for (std::size_t n = 0; n < names.size(); ++n) {
    const std::string expected = upf::renderJson(upf::ScenarioRunner().run(names[n]));
    for (int i = 0; i < 1000; ++i)
      REQUIRE(upf::renderJson(upf::ScenarioRunner().run(names[n])) == expected);
  }
}
