#include "upf/application/scenario_runner.hpp"
#include "upf/application/reconciliation.hpp"

namespace upf {
namespace {
RuleSet uplinkRules() {
  RuleSet rules; rules.sessionId = "session-1"; rules.version = 3;
  Pdr pdr; pdr.id = PdrId(10); pdr.precedence = 100; pdr.sourceInterface = N3;
  pdr.teid = Optional<std::uint32_t>(1001);
  pdr.uePrefix = IpPrefix::parse("10.10.0.0/24").value();
  pdr.protocol = Optional<TransportProtocol>(UDP);
  pdr.localPorts = Optional<PortRange>(PortRange(41000, 41000));
  pdr.remotePorts = Optional<PortRange>(PortRange(443, 443));
  pdr.qfi = Optional<std::uint8_t>(9); pdr.farId = FarId(20);
  pdr.qerId = Optional<QerId>(QerId(30)); pdr.urrId = Optional<UrrId>(UrrId(40));
  rules.pdrs.push_back(pdr);
  Far far; far.id = FarId(20); far.action = FORWARD;
  far.destination = Optional<Interface>(N6); far.operation = REMOVE; rules.fars.push_back(far);
  Qer qer; qer.id = QerId(30); qer.gate = OPEN; qer.rateBitsPerSecond = 10000000;
  qer.burstBytes = 20000; rules.qers.push_back(qer);
  Urr urr; urr.id = UrrId(40); urr.volumeThresholdBytes = 10000; rules.urrs.push_back(urr);
  return rules;
}
Packet uplinkPacket() {
  Packet packet; packet.packetId = "pkt-ul-001"; packet.sessionId = "session-1";
  packet.ingress = N3; packet.tunnel = Optional<TunnelMetadata>(TunnelMetadata(1001, 9));
  packet.sourceIp = IpAddress::parse("10.10.0.7").value();
  packet.destinationIp = IpAddress::parse("192.0.2.20").value();
  packet.protocol = UDP; packet.sourcePort = 41000; packet.destinationPort = 443;
  packet.payloadBytes = 1200; packet.arrivalTimeMicros = 0; return packet;
}
RuleSet downlinkRules() {
  RuleSet rules; rules.sessionId = "session-1"; rules.version = 3;
  Pdr pdr; pdr.id = PdrId(11); pdr.precedence = 100; pdr.sourceInterface = N6;
  pdr.uePrefix = IpPrefix::parse("10.10.0.0/24").value();
  pdr.protocol = Optional<TransportProtocol>(UDP);
  pdr.localPorts = Optional<PortRange>(PortRange(41000, 41000));
  pdr.remotePorts = Optional<PortRange>(PortRange(443, 443));
  pdr.farId = FarId(21); pdr.qerId = Optional<QerId>(QerId(31));
  pdr.urrId = Optional<UrrId>(UrrId(41)); rules.pdrs.push_back(pdr);
  Far far; far.id = FarId(21); far.action = FORWARD;
  far.destination = Optional<Interface>(N3); far.operation = ADD;
  far.tunnel = Optional<TunnelMetadata>(TunnelMetadata(2001, 9)); rules.fars.push_back(far);
  Qer qer; qer.id = QerId(31); qer.gate = OPEN; qer.rateBitsPerSecond = 10000000;
  qer.burstBytes = 20000; rules.qers.push_back(qer);
  Urr urr; urr.id = UrrId(41); urr.volumeThresholdBytes = 10000; rules.urrs.push_back(urr);
  return rules;
}
Packet downlinkPacket() {
  Packet packet; packet.packetId = "pkt-dl-001"; packet.sessionId = "session-1";
  packet.ingress = N6; packet.sourceIp = IpAddress::parse("192.0.2.20").value();
  packet.destinationIp = IpAddress::parse("10.10.0.7").value(); packet.protocol = UDP;
  packet.sourcePort = 443; packet.destinationPort = 41000; packet.payloadBytes = 900;
  packet.arrivalTimeMicros = 0; return packet;
}
} // namespace

std::vector<std::string> ScenarioRunner::scenarioNames() {
  std::vector<std::string> names;
  names.push_back("uplink-forward"); names.push_back("downlink-forward");
  names.push_back("pdr-precedence"); names.push_back("qer-rate-limit");
  names.push_back("urr-threshold"); names.push_back("adapter-failure");
  names.push_back("queue-full"); names.push_back("overload");
  names.push_back("recovery"); names.push_back("shutdown");
  return names;
}

RunResult ScenarioRunner::run(const std::string& name) const {
  RunResult result; result.runId = "upf-demo-001"; result.scenarioId = name;
  PacketPipeline pipeline;
  if (name == "downlink-forward") {
    pipeline.install(downlinkRules()); result.decisions.push_back(pipeline.process(downlinkPacket(), result.runId, name));
  } else if (name == "qer-rate-limit") {
    pipeline.install(uplinkRules());
    Packet first = uplinkPacket(); first.packetId = "pkt-rate-001"; first.payloadBytes = 19000;
    Packet second = uplinkPacket(); second.packetId = "pkt-rate-002"; second.payloadBytes = 2000;
    result.decisions.push_back(pipeline.process(first, result.runId, name));
    result.decisions.push_back(pipeline.process(second, result.runId, name));
  } else if (name == "urr-threshold") {
    pipeline.install(uplinkRules());
    Packet crossing = uplinkPacket(); crossing.packetId = "pkt-urr-001"; crossing.payloadBytes = 10000;
    result.decisions.push_back(pipeline.process(crossing, result.runId, name));
  } else if (name == "pdr-precedence") {
    RuleSet rules = uplinkRules();
    Pdr lowerPriority = rules.pdrs[0];
    lowerPriority.id = PdrId(11); lowerPriority.precedence = 200;
    rules.pdrs.push_back(lowerPriority);
    pipeline.install(rules);
    result.decisions.push_back(pipeline.process(uplinkPacket(), result.runId, name));
  } else if (name == "adapter-failure" || name == "recovery") {
    RuleService service;
    RuleSet rules = uplinkRules();
    service.update("update-3", rules);
    std::vector<AdapterResult> outcomes;
    outcomes.push_back(AdapterResult(TRANSIENT_FAILURE));
    outcomes.push_back(AdapterResult(SUCCESS));
    SimulatedDataplane adapter(outcomes);
    ReconciliationController reconciliation(service, adapter, 3);
    const ReconciliationStatus status = reconciliation.reconcile();
    pipeline.install(rules);
    result.decisions.push_back(pipeline.process(uplinkPacket(), result.runId, name));
    result.intendedVersion = rules.version; result.activeVersion = rules.version;
    result.appliedVersion = service.appliedVersion();
    result.convergence = status.converged ? CONVERGED : DIVERGENT;
  } else if (name == "overload") {
    CapacityLimits limits;
    limits.events = 0;
    pipeline = PacketPipeline(limits);
    pipeline.install(uplinkRules());
    result.decisions.push_back(pipeline.process(uplinkPacket(), result.runId, name));
  } else if (name == "queue-full" || name == "shutdown") {
    PacketDecision classified;
    const Packet packet = uplinkPacket();
    classified.runId = result.runId; classified.scenarioId = name;
    classified.packetId = packet.packetId; classified.sessionId = packet.sessionId;
    classified.ingress = packet.ingress; classified.rulesetVersion = 3;
    classified.terminalResult = name == "queue-full" ? PACKET_QUEUE_FULL : SHUTDOWN_DEADLINE;
    result.decisions.push_back(classified);
  } else {
    pipeline.install(uplinkRules()); result.decisions.push_back(pipeline.process(uplinkPacket(), result.runId, name));
  }
  result.n3Sink = pipeline.n3Sink(); result.n6Sink = pipeline.n6Sink();
  result.usageReports = pipeline.reports();
  result.observabilityDegraded = pipeline.observabilityDegraded();
  result.droppedEvents = pipeline.droppedEvents();
  if (name != "adapter-failure" && name != "recovery") {
    result.intendedVersion = 3; result.appliedVersion = 3; result.activeVersion = 3;
    result.convergence = CONVERGED;
  }
  return result;
}

} // namespace upf
