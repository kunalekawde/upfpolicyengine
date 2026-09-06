#include <catch2/catch.hpp>
#include "upf/application/scheduler.hpp"
#include "upf/application/packet_pipeline.hpp"
#include "tests/support/test_builders.hpp"

TEST_CASE("all scheduler queues remain bounded") {
  upf::Scheduler scheduler(upf::CapacityLimits(1, 1, 1));
  REQUIRE(scheduler.enqueueControl("c")); REQUIRE_FALSE(scheduler.enqueueControl("overflow"));
  REQUIRE(scheduler.enqueuePacket("p")); REQUIRE_FALSE(scheduler.enqueuePacket("overflow"));
  REQUIRE(scheduler.enqueueReconciliation("r"));
  REQUIRE_FALSE(scheduler.enqueueReconciliation("overflow"));
  REQUIRE(scheduler.pending() == 3);
}

TEST_CASE("shutdown stops admission and classifies remainder") {
  upf::Scheduler scheduler(upf::CapacityLimits(2, 2, 2));
  REQUIRE(scheduler.enqueuePacket("p1")); REQUIRE(scheduler.enqueueControl("c1"));
  scheduler.stopAdmission();
  REQUIRE_FALSE(scheduler.enqueuePacket("p2"));
  const std::vector<upf::ScheduledItem> remainder = scheduler.classifyRemainder();
  REQUIRE(remainder.size() == 2);
  REQUIRE(scheduler.pending() == 0);
}

TEST_CASE("result capacity rejects before QER and URR mutation") {
  upf::CapacityLimits limits;
  limits.results = 1;
  upf::PacketPipeline pipeline(limits);
  REQUIRE(pipeline.install(upf_test::basicRules()).ok());
  const upf::Packet packet = upf_test::uplinkPacket();
  REQUIRE(pipeline.process(packet, "r", "capacity").terminalResult == upf::FORWARDED_TO_N6);
  const std::uint64_t tokens = pipeline.runtime().qers.find(30)->second.tokensBytes;
  const std::uint64_t bytes = pipeline.runtime().urrs.find(40)->second.payloadBytes;
  REQUIRE(pipeline.process(packet, "r", "capacity").terminalResult == upf::CAPACITY_ERROR);
  REQUIRE(pipeline.runtime().qers.find(30)->second.tokensBytes == tokens);
  REQUIRE(pipeline.runtime().urrs.find(40)->second.payloadBytes == bytes);
  REQUIRE(pipeline.n6Sink().size() == 1);
}
