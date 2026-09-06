#include "upf/ports/packet_sink.hpp"
#include <vector>
namespace upf {
class InMemoryPacketSink : public PacketSink {
public:
  explicit InMemoryPacketSink(std::size_t capacity) : capacity_(capacity) {}
  bool accept(const Packet& packet) { if (packets_.size() >= capacity_) return false; packets_.push_back(packet); return true; }
  std::size_t size() const { return packets_.size(); }
private:
  std::size_t capacity_; std::vector<Packet> packets_;
};
} // namespace upf
