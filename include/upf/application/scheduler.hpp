#pragma once

#include "upf/domain/types.hpp"
#include <deque>
#include <string>
#include <vector>

namespace upf {

enum WorkKind { CONTROL_WORK, PACKET_WORK, RECONCILIATION_WORK };
struct ScheduledItem {
  ScheduledItem(WorkKind itemKind, const std::string& itemValue) : kind(itemKind), value(itemValue) {}
  WorkKind kind;
  std::string value;
};

class Scheduler {
public:
  explicit Scheduler(const CapacityLimits& limits) : limits_(limits), accepting_(true) {}
  bool enqueueControl(const std::string& value);
  bool enqueuePacket(const std::string& value);
  bool enqueueReconciliation(const std::string& value);
  std::vector<ScheduledItem> takeCycle();
  void stopAdmission() { accepting_ = false; }
  bool accepting() const { return accepting_; }
  std::vector<ScheduledItem> classifyRemainder();
  std::size_t pending() const;
private:
  static bool enqueue(std::deque<std::string>& queue, std::size_t capacity,
                      const std::string& value);
  CapacityLimits limits_;
  std::deque<std::string> control_;
  std::deque<std::string> packet_;
  std::deque<std::string> reconciliation_;
  bool accepting_;
};

} // namespace upf
