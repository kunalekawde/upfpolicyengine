#include <catch2/catch.hpp>
#include "upf/application/scenario_runner.hpp"
#include <nlohmann/json.hpp>

TEST_CASE("JSON output is one parseable run object") {
  const std::string text = upf::renderJson(upf::ScenarioRunner().run("uplink-forward"));
  const nlohmann::json value = nlohmann::json::parse(text);
  REQUIRE(value["schemaVersion"] == "1.0");
  REQUIRE(value["decisions"].size() == 1);
}
