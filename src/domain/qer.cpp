#include "upf/domain/engine.hpp"
#include <algorithm>

namespace upf {

QerEvaluation evaluateQer(const Qer& qer, const TokenBucketState& current,
                          std::uint64_t packetBytes, std::uint64_t nowMicros) {
  QerEvaluation result;
  result.next = current;
  result.failure = RATE_LIMITED;
  result.decision.applicable = true;
  result.decision.qerId = qer.id.value;
  result.decision.gate = qer.gate;
  result.decision.tokensBefore = current.tokensBytes;
  result.decision.packetChargeBytes = packetBytes;
  result.decision.tokensAfter = current.tokensBytes;
  if (qer.gate == CLOSED) {
    result.decision.outcome = "GATE_CLOSED";
    result.failure = GATE_CLOSED;
    return result;
  }
  if (nowMicros < current.lastRefillMicros) {
    result.decision.outcome = "ARITHMETIC_ERROR";
    result.failure = ARITHMETIC_ERROR;
    return result;
  }
  const std::uint64_t elapsed = nowMicros - current.lastRefillMicros;
  std::uint64_t product = 0;
  std::uint64_t numerator = 0;
  if (!checkedMultiply(elapsed, qer.rateBitsPerSecond, product) ||
      !checkedAdd(product, current.remainderBitMicros, numerator)) {
    result.decision.outcome = "ARITHMETIC_ERROR";
    result.failure = ARITHMETIC_ERROR;
    return result;
  }
  result.decision.refillBytes = numerator / 8000000ULL;
  result.next.remainderBitMicros = numerator % 8000000ULL;
  result.next.lastRefillMicros = nowMicros;
  const std::uint64_t room = qer.burstBytes > current.tokensBytes
      ? qer.burstBytes - current.tokensBytes : 0;
  const std::uint64_t added = std::min(room, result.decision.refillBytes);
  result.next.tokensBytes = current.tokensBytes + added;
  if (packetBytes > result.next.tokensBytes) {
    result.decision.tokensAfter = result.next.tokensBytes;
    result.decision.outcome = "RATE_LIMITED";
    return result;
  }
  result.next.tokensBytes -= packetBytes;
  result.decision.tokensAfter = result.next.tokensBytes;
  result.decision.allowed = true;
  result.decision.outcome = "ALLOW";
  return result;
}

} // namespace upf
