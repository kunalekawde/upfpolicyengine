#pragma once

#include "upf/domain/packet.hpp"
#include "upf/domain/rules.hpp"

namespace upf_test {
inline upf::Packet uplinkPacket() {
  upf::Packet packet;
  packet.packetId = "pkt-ul-001";
  packet.sessionId = "session-1";
  packet.ingress = upf::N3;
  packet.tunnel = upf::Optional<upf::TunnelMetadata>(upf::TunnelMetadata(1001, 9));
  packet.sourceIp = upf::IpAddress::parse("10.10.0.7").value();
  packet.destinationIp = upf::IpAddress::parse("192.0.2.20").value();
  packet.protocol = upf::UDP;
  packet.sourcePort = 41000;
  packet.destinationPort = 443;
  packet.payloadBytes = 1200;
  packet.arrivalTimeMicros = 0;
  return packet;
}

inline upf::RuleSet basicRules() {
  upf::RuleSet rules;
  rules.sessionId = "session-1";
  rules.version = 3;
  upf::Pdr pdr;
  pdr.id = upf::PdrId(10); pdr.precedence = 100; pdr.sourceInterface = upf::N3;
  pdr.teid = upf::Optional<std::uint32_t>(1001);
  pdr.uePrefix = upf::IpPrefix::parse("10.10.0.0/24").value();
  pdr.protocol = upf::Optional<upf::TransportProtocol>(upf::UDP);
  pdr.localPorts = upf::Optional<upf::PortRange>(upf::PortRange(41000, 41000));
  pdr.remotePorts = upf::Optional<upf::PortRange>(upf::PortRange(443, 443));
  pdr.qfi = upf::Optional<std::uint8_t>(9);
  pdr.farId = upf::FarId(20);
  pdr.qerId = upf::Optional<upf::QerId>(upf::QerId(30));
  pdr.urrId = upf::Optional<upf::UrrId>(upf::UrrId(40));
  rules.pdrs.push_back(pdr);
  upf::Far far;
  far.id = upf::FarId(20); far.action = upf::FORWARD;
  far.destination = upf::Optional<upf::Interface>(upf::N6); far.operation = upf::REMOVE;
  rules.fars.push_back(far);
  upf::Qer qer;
  qer.id = upf::QerId(30); qer.gate = upf::OPEN;
  qer.rateBitsPerSecond = 10000000; qer.burstBytes = 20000; qer.generation = 1;
  rules.qers.push_back(qer);
  upf::Urr urr;
  urr.id = upf::UrrId(40); urr.volumeThresholdBytes = 10000; urr.generation = 1;
  rules.urrs.push_back(urr);
  return rules;
}
} // namespace upf_test
