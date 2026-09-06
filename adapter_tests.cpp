#include <catch2/catch.hpp>
#include "upf/ports/dataplane_adapter.hpp"

TEST_CASE("adapter immediate and delayed success are idempotent") {
  std::vector<upf::AdapterResult> outcomes;
  outcomes.push_back(upf::AdapterResult(upf::SUCCESS));
  outcomes.push_back(upf::AdapterResult(upf::DELAYED_SUCCESS, 100));
  upf::SimulatedDataplane adapter(outcomes);
  REQUIRE(adapter.apply("u1", 1, 1).outcome == upf::SUCCESS);
  REQUIRE(adapter.apply("u1", 1, 2).outcome == upf::SUCCESS);
  REQUIRE(adapter.apply("u2", 2, 1).outcome == upf::DELAYED_SUCCESS);
  REQUIRE(adapter.completeDelayed("u2", 2, "u2", 2));
  REQUIRE(adapter.completeDelayed("u2", 2, "u2", 2));
  REQUIRE_FALSE(adapter.completeDelayed("old", 1, "u2", 2));
}
