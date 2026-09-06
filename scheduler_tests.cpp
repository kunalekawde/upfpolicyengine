#include <catch2/catch.hpp>
#include "upf/application/scheduler.hpp"

TEST_CASE("scheduler is FIFO and round robin") {
  upf::Scheduler scheduler(upf::CapacityLimits(2, 2, 2));
  REQUIRE(scheduler.enqueueControl("c1"));
  REQUIRE(scheduler.enqueueControl("c2"));
  REQUIRE_FALSE(scheduler.enqueueControl("c3"));
  REQUIRE(scheduler.enqueuePacket("p1"));
  REQUIRE(scheduler.enqueueReconciliation("r1"));
  const std::vector<upf::ScheduledItem> cycle = scheduler.takeCycle();
  REQUIRE(cycle.size() == 3);
  REQUIRE(cycle[0].value == "c1");
  REQUIRE(cycle[1].value == "p1");
  REQUIRE(cycle[2].value == "r1");
  REQUIRE(scheduler.takeCycle()[0].value == "c2");
}
