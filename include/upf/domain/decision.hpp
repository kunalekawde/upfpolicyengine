#pragma once

#include "upf/domain/packet.hpp"
#include <string>
#include <vector>

namespace upf {

enum TerminalResult {
  FORWARDED_TO_N3, FORWARDED_TO_N6, FAR_DROPPED, NO_MATCH, MALFORMED_PACKET,
  GATE_CLOSED, RATE_LIMITED, PACKET_QUEUE_FULL, SHUTDOWN_DEADLINE,
  ARITHMETIC_ERROR, CAPACITY_ERROR
};

struct CriterionResult {
  std::string name;
  bool applicable;
  bool matched;
  std::string actual;
  std::string expected;
};

struct MatchCandidate {
  std::uint32_t pdrId;
  std::uint32_t precedence;
  bool matched;
  std::vector<CriterionResult> criteria;
};

struct QerDecision {
  QerDecision() : applicable(false), qerId(0), gate(OPEN), tokensBefore(0), refillBytes(0),
      packetChargeBytes(0), tokensAfter(0), allowed(false) {}
  bool applicable;
  std::uint32_t qerId;
  GateStatus gate;
  std::uint64_t tokensBefore;
  std::uint64_t refillBytes;
  std::uint64_t packetChargeBytes;
  std::uint64_t tokensAfter;
  bool allowed;
  std::string outcome;
};

struct FarDecision {
  FarDecision() : applicable(false), farId(0), action(DROP), destination(N6) {}
  bool applicable;
  std::uint32_t farId;
  FarAction action;
  Interface destination;
  Optional<TunnelMetadata> before;
  Optional<TunnelMetadata> after;
};

struct UrrDecision {
  UrrDecision() : applicable(false), urrId(0), counted(false), packetsBefore(0), packetsAfter(0),
      bytesBefore(0), bytesAfter(0), thresholdReached(false) {}
  bool applicable;
  std::uint32_t urrId;
  bool counted;
  std::uint64_t packetsBefore;
  std::uint64_t packetsAfter;
  std::uint64_t bytesBefore;
  std::uint64_t bytesAfter;
  bool thresholdReached;
  std::string reportId;
};

struct PacketDecision {
  PacketDecision() : rulesetVersion(0), selectedPdrId(0), terminalResult(NO_MATCH),
      sinkPresent(false), sink(N6), transformedPresent(false), observabilityDegraded(false) {}
  std::string runId;
  std::string scenarioId;
  std::string packetId;
  std::string sessionId;
  std::uint64_t arrivalTimeMicros;
  Interface ingress;
  std::uint64_t rulesetVersion;
  std::string updateId;
  std::vector<MatchCandidate> candidates;
  std::uint32_t selectedPdrId;
  QerDecision qer;
  FarDecision far;
  UrrDecision urr;
  TerminalResult terminalResult;
  bool sinkPresent;
  Interface sink;
  bool transformedPresent;
  Packet transformedPacket;
  bool observabilityDegraded;
};

std::string terminalResultName(TerminalResult result);

} // namespace upf
