#include "upf/ports/event_sink.hpp"
#include <limits>
namespace upf {
BoundedEventSink::BoundedEventSink(std::size_t capacity) : capacity_(capacity), dropped_(0) {}

bool BoundedEventSink::emit(const std::string& event) {
  if (events_.size() >= capacity_) {
    if (dropped_ != std::numeric_limits<std::size_t>::max()) ++dropped_;
    return false;
  }
  events_.push_back(event);
  return true;
}

std::size_t BoundedEventSink::dropped() const { return dropped_; }
} // namespace upf
