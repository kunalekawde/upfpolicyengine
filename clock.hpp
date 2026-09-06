#pragma once
#include <cstdint>
namespace upf {
class Clock {
public:
  virtual ~Clock() {}
  virtual std::uint64_t nowMicros() const = 0;
};
} // namespace upf
