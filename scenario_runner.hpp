#pragma once

#include "upf/application/packet_pipeline.hpp"
#include <string>
#include <vector>

namespace upf {

struct RunResult {
  RunResult() : intendedVersion(0), appliedVersion(0), activeVersion(0),
                convergence(EMPTY), observabilityDegraded(false), droppedEvents(0) {}
  std::string runId;
  std::string scenarioId;
  std::vector<PacketDecision> decisions;
  std::vector<std::string> usageReports;
  std::vector<Packet> n3Sink;
  std::vector<Packet> n6Sink;
  std::uint64_t intendedVersion;
  std::uint64_t appliedVersion;
  std::uint64_t activeVersion;
  ConvergenceStatus convergence;
  bool observabilityDegraded;
  std::size_t droppedEvents;
};

class ScenarioRunner {
public:
  static std::vector<std::string> scenarioNames();
  RunResult run(const std::string& name) const;
};

std::string renderHuman(const RunResult& result);
std::string renderJson(const RunResult& result);
Result<RunResult> runFiles(const std::string& rulesPath, const std::string& packetsPath);

} // namespace upf
