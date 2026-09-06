#include "upf/ports/clock.hpp"
namespace upf {
class SimulatedClock : public Clock {
public:
  SimulatedClock() : now_(0) {}
  std::uint64_t nowMicros() const { return now_; }
  void advanceTo(std::uint64_t value) { if (value >= now_) now_ = value; }
private:
  std::uint64_t now_;
};
} // namespace upf
