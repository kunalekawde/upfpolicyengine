#pragma once
#include <cstddef>
#include <vector>
#include <string>
namespace upf {
class EventSink {
public:
  virtual ~EventSink() {}
  virtual bool emit(const std::string& event) = 0;
  virtual std::size_t dropped() const = 0;
};

class BoundedEventSink : public EventSink {
public:
  explicit BoundedEventSink(std::size_t capacity);
  bool emit(const std::string& event);
  std::size_t dropped() const;
private:
  std::size_t capacity_;
  std::size_t dropped_;
  std::vector<std::string> events_;
};
} // namespace upf
