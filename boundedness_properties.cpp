#include <catch2/catch.hpp>
#include "upf/application/scheduler.hpp"

TEST_CASE("queue sizes never exceed configured bounds") {
  for (std::size_t capacity = 1; capacity <= 64; ++capacity) {
    upf::Scheduler scheduler(upf::CapacityLimits(capacity, capacity, capacity));
    for (std::size_t i = 0; i < capacity * 2; ++i) scheduler.enqueuePacket(std::to_string(i));
    REQUIRE(scheduler.pending() == capacity);
  }
}
