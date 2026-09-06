#include "upf/application/scenario_runner.hpp"
#include <nlohmann/json.hpp>

namespace upf {
namespace {
const char* interfaceName(Interface value) { return value == N3 ? "N3" : "N6"; }

nlohmann::json tunnelJson(const Optional<TunnelMetadata>& tunnel) {
  if (!tunnel.hasValue()) return nlohmann::json();
  return nlohmann::json{{"teid", tunnel.value().teid}, {"qfi", tunnel.value().qfi}};
}

nlohmann::json packetJson(const Packet& packet) {
  nlohmann::json value{{"packetId", packet.packetId}, {"sessionId", packet.sessionId},
      {"ingress", interfaceName(packet.ingress)}, {"sourceIp", packet.sourceIp.toString()},
      {"destinationIp", packet.destinationIp.toString()},
      {"protocol", packet.protocol == TCP ? "TCP" : packet.protocol == UDP ? "UDP" :
          packet.protocol == ICMP ? "ICMP" : "ICMPV6"},
      {"sourcePort", packet.sourcePort}, {"destinationPort", packet.destinationPort},
      {"payloadBytes", packet.payloadBytes}, {"arrivalTimeMicros", packet.arrivalTimeMicros}};
  if (packet.tunnel.hasValue()) value["tunnel"] = tunnelJson(packet.tunnel);
  return value;
}

nlohmann::json candidateJson(const MatchCandidate& candidate) {
  nlohmann::json criteria = nlohmann::json::array();
  for (std::size_t i = 0; i < candidate.criteria.size(); ++i) {
    const CriterionResult& item = candidate.criteria[i];
    criteria.push_back({{"name", item.name}, {"applicable", item.applicable},
        {"matched", item.applicable ? nlohmann::json(item.matched) : nlohmann::json()},
        {"actual", item.actual.empty() ? nlohmann::json() : nlohmann::json(item.actual)},
        {"expected", item.expected.empty() ? nlohmann::json() : nlohmann::json(item.expected)}});
  }
  return nlohmann::json{{"pdrId", candidate.pdrId}, {"matched", candidate.matched},
                        {"criteria", criteria}};
}

nlohmann::json decisionJson(const PacketDecision& d) {
  nlohmann::json candidates = nlohmann::json::array();
  for (std::size_t i = 0; i < d.candidates.size(); ++i) candidates.push_back(candidateJson(d.candidates[i]));
  const nlohmann::json selected = d.selectedPdrId == 0 ? nlohmann::json() : nlohmann::json(d.selectedPdrId);
  const nlohmann::json qerId = d.qer.applicable ? nlohmann::json(d.qer.qerId) : nlohmann::json();
  const nlohmann::json farId = d.far.applicable ? nlohmann::json(d.far.farId) : nlohmann::json();
  const nlohmann::json urrId = d.urr.applicable ? nlohmann::json(d.urr.urrId) : nlohmann::json();
  const nlohmann::json nullableZero = nlohmann::json();
  nlohmann::json qer{{"qerId", qerId}, {"gate", d.qer.applicable ? (d.qer.gate == OPEN ? "OPEN" : "CLOSED") : "NOT_APPLICABLE"},
      {"tokensBefore", d.qer.applicable ? nlohmann::json(d.qer.tokensBefore) : nullableZero},
      {"refillBytes", d.qer.applicable ? nlohmann::json(d.qer.refillBytes) : nullableZero},
      {"packetChargeBytes", d.qer.applicable ? nlohmann::json(d.qer.packetChargeBytes) : nullableZero},
      {"tokensAfter", d.qer.applicable ? nlohmann::json(d.qer.tokensAfter) : nullableZero},
      {"outcome", d.qer.applicable ? d.qer.outcome : "NOT_APPLICABLE"}};
  nlohmann::json far{{"farId", farId},
      {"action", d.far.applicable ? (d.far.action == FORWARD ? "FORWARD" : "DROP") : "NOT_APPLICABLE"},
      {"destination", d.far.applicable ? (d.far.action == DROP ? "NONE" : interfaceName(d.far.destination)) : "NOT_APPLICABLE"},
      {"tunnelBefore", tunnelJson(d.far.before)}, {"tunnelAfter", tunnelJson(d.far.after)}};
  nlohmann::json urr{{"urrId", urrId}, {"counted", d.urr.counted},
      {"packetsBefore", d.urr.applicable ? nlohmann::json(d.urr.packetsBefore) : nullableZero},
      {"packetsAfter", d.urr.applicable ? nlohmann::json(d.urr.packetsAfter) : nullableZero},
      {"bytesBefore", d.urr.applicable ? nlohmann::json(d.urr.bytesBefore) : nullableZero},
      {"bytesAfter", d.urr.applicable ? nlohmann::json(d.urr.bytesAfter) : nullableZero},
      {"thresholdReached", d.urr.applicable ? nlohmann::json(d.urr.thresholdReached) : nullableZero},
      {"usageReportId", d.urr.reportId.empty() ? nlohmann::json() : nlohmann::json(d.urr.reportId)}};
  return nlohmann::json{{"packetId", d.packetId}, {"sessionId", d.sessionId},
      {"arrivalTimeMicros", d.arrivalTimeMicros}, {"ingress", interfaceName(d.ingress)},
      {"rulesetVersion", d.rulesetVersion}, {"updateId", d.updateId.empty() ? nlohmann::json() : nlohmann::json(d.updateId)},
      {"candidates", candidates}, {"selectedPdrId", selected}, {"qer", qer}, {"far", far}, {"urr", urr},
      {"terminalResult", terminalResultName(d.terminalResult)}, {"sink", d.sinkPresent ? interfaceName(d.sink) : "NONE"},
      {"transformedPacket", d.transformedPresent ? packetJson(d.transformedPacket) : nlohmann::json()},
      {"observabilityDegraded", d.observabilityDegraded}};
}
} // namespace

std::string renderJson(const RunResult& result) {
  nlohmann::json root;
  root["schemaVersion"] = "1.0"; root["runId"] = result.runId; root["scenarioId"] = result.scenarioId;
  root["disclaimer"] = "Simulation-only educational demo; not a conformant or production PFCP, GTP-U, or UPF implementation.";
  root["decisions"] = nlohmann::json::array(); root["events"] = nlohmann::json::array();
  root["usageReports"] = nlohmann::json::array(); root["n3Sink"] = nlohmann::json::array(); root["n6Sink"] = nlohmann::json::array();
  for (std::size_t i = 0; i < result.decisions.size(); ++i) {
    const PacketDecision& decision = result.decisions[i];
    root["decisions"].push_back(decisionJson(decision));
    root["events"].push_back({{"schemaVersion", "1.0"}, {"runId", result.runId}, {"scenarioId", result.scenarioId},
        {"sequence", i + 1}, {"simulatedTimeMicros", decision.arrivalTimeMicros}, {"component", "packet-pipeline"},
        {"severity", "INFO"}, {"kind", "terminal-result"}, {"sessionId", decision.sessionId},
        {"packetId", decision.packetId}, {"updateId", nullptr}, {"rulesetVersion", decision.rulesetVersion},
        {"data", {{"terminalResult", terminalResultName(decision.terminalResult)}}}});
    if (!decision.urr.reportId.empty()) {
      root["usageReports"].push_back({{"reportId", decision.urr.reportId}, {"sessionId", decision.sessionId},
          {"urrId", decision.urr.urrId}, {"generation", 1}, {"rulesetVersion", decision.rulesetVersion},
          {"packets", decision.urr.packetsAfter}, {"payloadBytes", decision.urr.bytesAfter},
          {"thresholdBytes", decision.urr.bytesAfter}});
    }
  }
  for (std::size_t i = 0; i < result.n3Sink.size(); ++i) root["n3Sink"].push_back(packetJson(result.n3Sink[i]));
  for (std::size_t i = 0; i < result.n6Sink.size(); ++i) root["n6Sink"].push_back(packetJson(result.n6Sink[i]));
  const char* convergence = result.convergence == CONVERGED ? "CONVERGED" : result.convergence == EMPTY ? "EMPTY" : "DIVERGENT";
  root["state"] = {{"intendedVersion", result.intendedVersion == 0 ? nlohmann::json() : nlohmann::json(result.intendedVersion)},
      {"appliedVersion", result.appliedVersion == 0 ? nlohmann::json() : nlohmann::json(result.appliedVersion)},
      {"activeVersion", result.activeVersion == 0 ? nlohmann::json() : nlohmann::json(result.activeVersion)},
      {"convergence", convergence}};
  root["observabilityDegraded"] = result.observabilityDegraded;
  root["droppedEventCount"] = result.droppedEvents;
  return root.dump();
}
} // namespace upf
