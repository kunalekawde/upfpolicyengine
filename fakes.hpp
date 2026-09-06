#pragma once

#include "upf/ports/clock.hpp"
#include "upf/ports/dataplane_adapter.hpp"
#include "upf/ports/event_sink.hpp"
#include "upf/ports/packet_sink.hpp"

namespace upf_test {
class FakeClock : public upf::Clock {
public:
  explicit FakeClock(std::uint64_t now = 0) : now_(now) {}
  std::uint64_t nowMicros() const { return now_; }
  void set(std::uint64_t now) { now_ = now; }
private:
  std::uint64_t now_;
};
} // namespace upf_test
