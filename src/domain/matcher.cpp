#include "upf/domain/decision.hpp"
#include "upf/domain/rules.hpp"
#include <sstream>

namespace upf {

static CriterionResult criterion(const std::string& name, bool applicable, bool matched,
                                 const std::string& actual, const std::string& expected) {
  CriterionResult value;
  value.name = name; value.applicable = applicable; value.matched = matched;
  value.actual = actual; value.expected = expected;
  return value;
}

MatchCandidate evaluateCandidate(const Pdr& pdr, const Packet& packet) {
  MatchCandidate result;
  result.pdrId = pdr.id.value; result.precedence = pdr.precedence; result.matched = true;
  const bool ingress = pdr.sourceInterface == packet.ingress;
  result.criteria.push_back(criterion("ingress", true, ingress, packet.ingress == N3 ? "N3" : "N6",
                                      pdr.sourceInterface == N3 ? "N3" : "N6"));
  result.matched = result.matched && ingress;
  const bool tunnel = !pdr.teid.hasValue() || (packet.tunnel.hasValue() &&
      packet.tunnel.value().teid == pdr.teid.value());
  result.criteria.push_back(criterion("tunnel", pdr.teid.hasValue(), tunnel, "semantic", "configured"));
  result.matched = result.matched && tunnel;
  const IpAddress& ue = packet.ingress == N3 ? packet.sourceIp : packet.destinationIp;
  const bool prefix = pdr.uePrefix.contains(ue);
  result.criteria.push_back(criterion("prefix", true, prefix, ue.toString(), "UE prefix"));
  result.matched = result.matched && prefix;
  const bool protocol = !pdr.protocol.hasValue() || pdr.protocol.value() == packet.protocol;
  result.criteria.push_back(criterion("protocol", pdr.protocol.hasValue(), protocol, "packet", "configured"));
  result.matched = result.matched && protocol;
  const std::uint16_t local = packet.ingress == N3 ? packet.sourcePort : packet.destinationPort;
  const std::uint16_t remote = packet.ingress == N3 ? packet.destinationPort : packet.sourcePort;
  const bool localMatch = !pdr.localPorts.hasValue() || pdr.localPorts.value().contains(local);
  const bool remoteMatch = !pdr.remotePorts.hasValue() || pdr.remotePorts.value().contains(remote);
  result.criteria.push_back(criterion("localPort", pdr.localPorts.hasValue(), localMatch, "packet", "range"));
  result.criteria.push_back(criterion("remotePort", pdr.remotePorts.hasValue(), remoteMatch, "packet", "range"));
  result.matched = result.matched && localMatch && remoteMatch;
  const bool qfi = !pdr.qfi.hasValue() || (packet.tunnel.hasValue() &&
      packet.tunnel.value().qfi == pdr.qfi.value());
  result.criteria.push_back(criterion("qfi", pdr.qfi.hasValue(), qfi, "packet", "configured"));
  result.matched = result.matched && qfi;
  return result;
}

} // namespace upf
