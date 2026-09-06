#pragma once

#include "upf/domain/decision.hpp"
#include "upf/domain/rules.hpp"
#include "upf/domain/runtime_state.hpp"

namespace upf {

struct QerEvaluation {
  QerDecision decision;
  TokenBucketState next;
  TerminalResult failure;
};
QerEvaluation evaluateQer(const Qer& qer, const TokenBucketState& current,
                          std::uint64_t packetBytes, std::uint64_t nowMicros);

struct FarEvaluation {
  FarEvaluation() : valid(false), forwarded(false), destination(N6) {}
  bool valid;
  bool forwarded;
  Interface destination;
  Packet packet;
};
FarEvaluation evaluateFar(const Far& far, const Packet& packet);

struct UrrEvaluation {
  UrrEvaluation() : reportGenerated(false), overflow(false) {}
  UsageCounterState next;
  UrrDecision decision;
  bool reportGenerated;
  bool overflow;
};
UrrEvaluation evaluateUrr(const Urr& urr, const UsageCounterState& current,
                          std::uint64_t payloadBytes, const std::string& sessionId,
                          std::uint64_t rulesetVersion);

} // namespace upf
