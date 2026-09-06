#pragma once

#include "upf/domain/decision.hpp"
#include "upf/domain/engine.hpp"
#include <memory>
#include <vector>

namespace upf {

MatchCandidate evaluateCandidate(const Pdr& pdr, const Packet& packet);

class PacketPipeline {
public:
  explicit PacketPipeline(const CapacityLimits& limits = CapacityLimits());
  Result<void> install(const RuleSet& rules);
  PacketDecision process(const Packet& packet, const std::string& runId,
                         const std::string& scenarioId);
  const std::vector<Packet>& n3Sink() const { return n3Sink_; }
  const std::vector<Packet>& n6Sink() const { return n6Sink_; }
  const std::vector<std::string>& reports() const { return reports_; }
  std::size_t droppedEvents() const { return droppedEvents_; }
  bool observabilityDegraded() const { return droppedEvents_ != 0; }
  const RuntimeState& runtime() const { return runtime_; }
private:
  bool validPacket(const Packet& packet) const;
  PacketDecision retain(PacketDecision decision);
  CapacityLimits limits_;
  std::shared_ptr<const RuleSet> rules_;
  RuntimeState runtime_;
  std::vector<Packet> n3Sink_;
  std::vector<Packet> n6Sink_;
  std::vector<PacketDecision> decisions_;
  std::vector<std::string> reports_;
  std::vector<std::string> events_;
  std::size_t droppedEvents_;
};

} // namespace upf
