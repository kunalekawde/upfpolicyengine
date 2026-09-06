#pragma once
#include "upf/domain/packet.hpp"
#include <cstddef>
namespace upf {
class PacketSink {
public:
  virtual ~PacketSink() {}
  virtual bool accept(const Packet& packet) = 0;
  virtual std::size_t size() const = 0;
};
} // namespace upf
