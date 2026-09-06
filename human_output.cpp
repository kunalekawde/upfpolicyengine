#include "upf/application/scenario_runner.hpp"
#include <sstream>

namespace upf {
std::string renderHuman(const RunResult& result) {
  std::ostringstream out;
  out << "Simulation-only educational demo; not a conformant or production PFCP, GTP-U, or UPF implementation.\n";
  out << "Run: " << result.runId << "\nScenario: " << result.scenarioId << "\n";
  for (std::size_t i = 0; i < result.decisions.size(); ++i) {
    const PacketDecision& d = result.decisions[i];
    out << "Packet: " << d.packetId << "\nSession: " << d.sessionId
        << "\nIngress: " << (d.ingress == N3 ? "N3" : "N6")
        << "\nSimulated time: " << d.arrivalTimeMicros
        << "\nRuleset version: " << d.rulesetVersion << "\nPDR candidates:\n";
    for (std::size_t candidateIndex = 0; candidateIndex < d.candidates.size(); ++candidateIndex) {
      const MatchCandidate& candidate = d.candidates[candidateIndex];
      out << "  PDR " << candidate.pdrId << ": " << (candidate.matched ? "MATCH" : "NO_MATCH") << "\n";
      for (std::size_t criterionIndex = 0; criterionIndex < candidate.criteria.size(); ++criterionIndex) {
        const CriterionResult& criterion = candidate.criteria[criterionIndex];
        out << "    " << criterion.name << ": "
            << (criterion.applicable ? (criterion.matched ? "MATCH" : "MISMATCH") : "NOT_APPLICABLE") << "\n";
      }
    }
    out << "PDR: ";
    if (d.selectedPdrId == 0) out << "none"; else out << d.selectedPdrId;
    out << "\nQER: " << d.qer.outcome;
    if (d.qer.applicable) out << " (tokens " << d.qer.tokensBefore << " + " << d.qer.refillBytes
                              << " - " << d.qer.packetChargeBytes << " = " << d.qer.tokensAfter << ')';
    out << "\nFAR: ";
    if (!d.far.applicable) out << "NOT_APPLICABLE";
    else out << (d.far.action == FORWARD ? "FORWARD" : "DROP");
    out << "\nURR packets: " << d.urr.packetsBefore << " -> " << d.urr.packetsAfter
      << "\nURR bytes: " << d.urr.bytesBefore << " -> " << d.urr.bytesAfter
      << "\nURR report: " << (d.urr.reportId.empty() ? "none" : d.urr.reportId)
      << "\nSink: " << (d.sinkPresent ? (d.sink == N3 ? "N3" : "N6") : "none");
    out << "\nResult: " << terminalResultName(d.terminalResult) << "\n";
  }
  return out.str();
}
} // namespace upf
