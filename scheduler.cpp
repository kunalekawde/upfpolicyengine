#include "upf/application/scheduler.hpp"

namespace upf {

bool Scheduler::enqueue(std::deque<std::string>& queue, std::size_t capacity,
                        const std::string& value) {
  if (queue.size() >= capacity) return false;
  queue.push_back(value);
  return true;
}

bool Scheduler::enqueueControl(const std::string& value) {
  return accepting_ && enqueue(control_, limits_.controlQueue, value);
}
bool Scheduler::enqueuePacket(const std::string& value) {
  return accepting_ && enqueue(packet_, limits_.packetQueue, value);
}
bool Scheduler::enqueueReconciliation(const std::string& value) {
  return accepting_ && enqueue(reconciliation_, limits_.reconciliationQueue, value);
}

std::vector<ScheduledItem> Scheduler::takeCycle() {
  std::vector<ScheduledItem> result;
  if (!control_.empty()) {
    result.push_back(ScheduledItem(CONTROL_WORK, control_.front()));
    control_.pop_front();
  }
  if (!packet_.empty()) {
    result.push_back(ScheduledItem(PACKET_WORK, packet_.front()));
    packet_.pop_front();
  }
  if (!reconciliation_.empty()) {
    result.push_back(ScheduledItem(RECONCILIATION_WORK, reconciliation_.front()));
    reconciliation_.pop_front();
  }
  return result;
}

std::size_t Scheduler::pending() const {
  return control_.size() + packet_.size() + reconciliation_.size();
}

std::vector<ScheduledItem> Scheduler::classifyRemainder() {
  std::vector<ScheduledItem> result;
  while (!control_.empty()) {
    result.push_back(ScheduledItem(CONTROL_WORK, control_.front())); control_.pop_front();
  }
  while (!packet_.empty()) {
    result.push_back(ScheduledItem(PACKET_WORK, packet_.front())); packet_.pop_front();
  }
  while (!reconciliation_.empty()) {
    result.push_back(ScheduledItem(RECONCILIATION_WORK, reconciliation_.front()));
    reconciliation_.pop_front();
  }
  return result;
}

} // namespace upf
