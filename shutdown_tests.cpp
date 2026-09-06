#include <catch2/catch.hpp>
#include "upf/application/scenario_runner.hpp"
#include "upf/application/scheduler.hpp"
#include "upf/ports/event_sink.hpp"
#include <algorithm>

TEST_CASE("shutdown rejects admission and classifies all accepted work") {
  upf::Scheduler scheduler(upf::CapacityLimits(2, 2, 2));
  REQUIRE(scheduler.enqueueControl("update"));
  REQUIRE(scheduler.enqueuePacket("packet"));
  scheduler.stopAdmission();
  REQUIRE_FALSE(scheduler.enqueueControl("late"));
  REQUIRE(scheduler.classifyRemainder().size() == 2);
  REQUIRE(scheduler.pending() == 0);
  REQUIRE(upf::ScenarioRunner().run("shutdown").decisions[0].terminalResult ==
          upf::SHUTDOWN_DEADLINE);
}

TEST_CASE("event exhaustion is bounded and does not change terminal scenarios") {
  upf::BoundedEventSink events(1);
  REQUIRE(events.emit("first"));
  REQUIRE_FALSE(events.emit("second"));
  REQUIRE(events.dropped() == 1);
  REQUIRE(upf::ScenarioRunner().run("queue-full").decisions[0].terminalResult ==
          upf::PACKET_QUEUE_FULL);
}

TEST_CASE("event exhaustion marks decisions and run output degraded") {
  const upf::RunResult result = upf::ScenarioRunner().run("overload");
  REQUIRE(result.observabilityDegraded);
  REQUIRE(result.droppedEvents > 0);
  REQUIRE(result.decisions.size() == 1);
  REQUIRE(result.decisions[0].observabilityDegraded);
  REQUIRE(result.decisions[0].terminalResult == upf::FORWARDED_TO_N6);
}

TEST_CASE("recovery and overload scenarios are independently selectable") {
  const std::vector<std::string> names = upf::ScenarioRunner::scenarioNames();
  REQUIRE(std::find(names.begin(), names.end(), "overload") != names.end());
  REQUIRE(std::find(names.begin(), names.end(), "recovery") != names.end());
  REQUIRE(upf::ScenarioRunner().run("recovery").convergence == upf::CONVERGED);
}
