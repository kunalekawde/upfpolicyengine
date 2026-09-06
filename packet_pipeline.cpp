#include "upf/application/packet_pipeline.hpp"
#include <algorithm>
#include <limits>

namespace upf {

PacketPipeline::PacketPipeline(const CapacityLimits& limits) : limits_(limits), droppedEvents_(0) {}

PacketDecision PacketPipeline::retain(PacketDecision decision) {
  if (events_.size() < limits_.events)
    events_.push_back(terminalResultName(decision.terminalResult));
  else {
    decision.observabilityDegraded = true;
    if (droppedEvents_ != std::numeric_limits<std::size_t>::max()) ++droppedEvents_;
  }
  decisions_.push_back(decision);
  return decision;
}

Result<void> PacketPipeline::install(const RuleSet& rules) {
  Result<void> valid = validateRuleSet(rules, limits_);
  if (!valid.ok()) return valid;
  rules_.reset(new RuleSet(rules));
  RuntimeState migrated;
  for (std::size_t i = 0; i < rules.qers.size(); ++i) {
    const Qer& qer = rules.qers[i];
    if (runtime_.qerGenerations[qer.id.value] == qer.generation &&
        runtime_.qers.find(qer.id.value) != runtime_.qers.end())
      migrated.qers[qer.id.value] = runtime_.qers[qer.id.value];
    else {
      TokenBucketState state;
      state.tokensBytes = qer.burstBytes;
      migrated.qers[qer.id.value] = state;
    }
    migrated.qerGenerations[qer.id.value] = qer.generation;
  }
  for (std::size_t i = 0; i < rules.urrs.size(); ++i) {
    const Urr& urr = rules.urrs[i];
    if (runtime_.urrGenerations[urr.id.value] == urr.generation &&
        runtime_.urrs.find(urr.id.value) != runtime_.urrs.end())
      migrated.urrs[urr.id.value] = runtime_.urrs[urr.id.value];
    else
      migrated.urrs[urr.id.value] = UsageCounterState();
    migrated.urrGenerations[urr.id.value] = urr.generation;
  }
  runtime_ = migrated;
  return Result<void>::success();
}

bool PacketPipeline::validPacket(const Packet& packet) const {
  if (packet.packetId.empty() || packet.sessionId.empty()) return false;
  if (packet.sourceIp.family() != packet.destinationIp.family()) return false;
  if (packet.ingress == N3) return packet.tunnel.hasValue() && packet.tunnel.value().valid();
  return !packet.tunnel.hasValue();
}

PacketDecision PacketPipeline::process(const Packet& packet, const std::string& runId,
                                       const std::string& scenarioId) {
  PacketDecision decision;
  decision.runId = runId; decision.scenarioId = scenarioId; decision.packetId = packet.packetId;
  decision.sessionId = packet.sessionId; decision.arrivalTimeMicros = packet.arrivalTimeMicros;
  decision.ingress = packet.ingress;
  if (rules_) decision.rulesetVersion = rules_->version;
  if (decisions_.size() >= limits_.results) {
    decision.terminalResult = CAPACITY_ERROR;
    return decision;
  }
  if (!rules_ || !validPacket(packet) || packet.sessionId != rules_->sessionId) {
    decision.terminalResult = rules_ ? MALFORMED_PACKET : NO_MATCH;
    return retain(decision);
  }
  const Pdr* winner = 0;
  for (std::size_t i = 0; i < rules_->pdrs.size(); ++i) {
    MatchCandidate candidate = evaluateCandidate(rules_->pdrs[i], packet);
    decision.candidates.push_back(candidate);
    if (candidate.matched && (winner == 0 || rules_->pdrs[i].precedence < winner->precedence))
      winner = &rules_->pdrs[i];
  }
  std::sort(decision.candidates.begin(), decision.candidates.end(),
            [](const MatchCandidate& a, const MatchCandidate& b) { return a.pdrId < b.pdrId; });
  if (winner == 0) { decision.terminalResult = NO_MATCH; return retain(decision); }
  decision.selectedPdrId = winner->id.value;

  TokenBucketState nextTokens;
  bool tokenChange = false;
  if (winner->qerId.hasValue()) {
    const Qer* qer = findQer(*rules_, winner->qerId.value());
    QerEvaluation evaluation = evaluateQer(*qer, runtime_.qers[qer->id.value],
                                           packet.payloadBytes, packet.arrivalTimeMicros);
    decision.qer = evaluation.decision;
    if (!evaluation.decision.allowed) {
      if (evaluation.failure == RATE_LIMITED) runtime_.qers[qer->id.value] = evaluation.next;
      decision.terminalResult = evaluation.failure;
      return retain(decision);
    }
    nextTokens = evaluation.next;
    tokenChange = true;
  } else {
    decision.qer.outcome = "NOT_APPLICABLE";
    decision.qer.allowed = true;
  }

  const Far* far = findFar(*rules_, winner->farId);
  const FarEvaluation farEvaluation = evaluateFar(*far, packet);
  if (!farEvaluation.valid) { decision.terminalResult = MALFORMED_PACKET; return retain(decision); }
  decision.far.applicable = true; decision.far.farId = far->id.value;
  decision.far.action = far->action; decision.far.before = packet.tunnel;
  decision.far.after = farEvaluation.packet.tunnel; decision.far.destination = farEvaluation.destination;

  UrrEvaluation usage;
  bool usageChange = false;
  if (winner->urrId.hasValue()) {
    const Urr* urr = findUrr(*rules_, winner->urrId.value());
    usage = evaluateUrr(*urr, runtime_.urrs[urr->id.value], packet.payloadBytes,
                        rules_->sessionId, rules_->version);
    decision.urr = usage.decision;
    if (usage.overflow) { decision.terminalResult = ARITHMETIC_ERROR; return retain(decision); }
    if (usage.reportGenerated && reports_.size() >= limits_.reports) {
      decision.terminalResult = CAPACITY_ERROR; return retain(decision);
    }
    usageChange = true;
  }
  std::vector<Packet>* sink = farEvaluation.destination == N3 ? &n3Sink_ : &n6Sink_;
  if (farEvaluation.forwarded && sink->size() >= limits_.sinkPackets) {
    decision.terminalResult = CAPACITY_ERROR; return retain(decision);
  }
  if (tokenChange) runtime_.qers[winner->qerId.value().value] = nextTokens;
  if (usageChange) {
    runtime_.urrs[winner->urrId.value().value] = usage.next;
    if (usage.reportGenerated) reports_.push_back(usage.decision.reportId);
  }
  if (far->action == DROP) {
    decision.terminalResult = FAR_DROPPED;
  } else {
    sink->push_back(farEvaluation.packet);
    decision.transformedPresent = true;
    decision.transformedPacket = farEvaluation.packet;
    decision.sinkPresent = true;
    decision.sink = farEvaluation.destination;
    decision.terminalResult = farEvaluation.destination == N3 ? FORWARDED_TO_N3 : FORWARDED_TO_N6;
  }
  return retain(decision);
}

} // namespace upf
