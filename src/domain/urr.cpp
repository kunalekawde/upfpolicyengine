#include "upf/domain/engine.hpp"
#include <sstream>

namespace upf {

UrrEvaluation evaluateUrr(const Urr& urr, const UsageCounterState& current,
                          std::uint64_t payloadBytes, const std::string& sessionId,
                          std::uint64_t rulesetVersion) {
  UrrEvaluation result;
  result.next = current;
  result.decision.applicable = true;
  result.decision.urrId = urr.id.value;
  result.decision.counted = true;
  result.decision.packetsBefore = current.packets;
  result.decision.bytesBefore = current.payloadBytes;
  if (!checkedAdd(current.packets, 1, result.next.packets) ||
      !checkedAdd(current.payloadBytes, payloadBytes, result.next.payloadBytes)) {
    result.overflow = true;
    result.next = current;
    return result;
  }
  result.decision.packetsAfter = result.next.packets;
  result.decision.bytesAfter = result.next.payloadBytes;
  if (!current.reportEmitted && result.next.payloadBytes >= urr.volumeThresholdBytes) {
    result.reportGenerated = true;
    result.next.reportEmitted = true;
    std::ostringstream id;
    id << sessionId << "-urr-" << urr.id.value << "-g" << urr.generation
       << "-v" << rulesetVersion;
    result.decision.reportId = id.str();
    result.decision.thresholdReached = true;
  }
  return result;
}

} // namespace upf
