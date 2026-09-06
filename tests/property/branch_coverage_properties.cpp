#include <catch2/catch.hpp>
#include "tests/support/test_builders.hpp"
#include "upf/application/packet_pipeline.hpp"
#include "upf/application/reconciliation.hpp"
#include "upf/application/rule_service.hpp"
#include "upf/application/scheduler.hpp"
#include "upf/domain/engine.hpp"

TEST_CASE("address parser rejects every invalid prefix form") {
  REQUIRE(upf::IpAddress::parse("2001:db8::1").value().toString() == "2001:db8::1");
  REQUIRE(upf::IpAddress::parse("192.0.2.1").value() == upf::IpAddress::parse("192.0.2.1").value());
  REQUIRE_FALSE(upf::IpPrefix::parse("192.0.2.0").ok());
  REQUIRE_FALSE(upf::IpPrefix::parse("bad/24").ok());
  REQUIRE_FALSE(upf::IpPrefix::parse("192.0.2.0/no").ok());
  REQUIRE_FALSE(upf::IpPrefix::parse("192.0.2.0/33").ok());
  REQUIRE_FALSE(upf::IpPrefix::parse("192.0.2.1/24").ok());
  REQUIRE(upf::IpPrefix::parse("0.0.0.0/0").value().contains(upf::IpAddress::parse("203.0.113.9").value()));
  REQUIRE_FALSE(upf::IpPrefix::parse("2001:db8::/33").value().contains(upf::IpAddress::parse("3001:db8::1").value()));
}

TEST_CASE("ruleset validation covers scalar reference capacity and direction failures") {
  upf::RuleSet rules = upf_test::basicRules();
  upf::CapacityLimits limits;
  rules.sessionId.clear(); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.version = 0; REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  limits.rulesPerType = 0; REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); limits.rulesPerType = 256;
  rules.pdrs[0].id = upf::PdrId(); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.pdrs.push_back(rules.pdrs[0]); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.fars.push_back(rules.fars[0]); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.qers.push_back(rules.qers[0]); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.urrs.push_back(rules.urrs[0]); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.pdrs[0].qerId = upf::Optional<upf::QerId>(upf::QerId(999)); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.pdrs[0].urrId = upf::Optional<upf::UrrId>(upf::UrrId(999)); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.pdrs[0].localPorts = upf::Optional<upf::PortRange>(upf::PortRange(2, 1)); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.pdrs[0].remotePorts = upf::Optional<upf::PortRange>(upf::PortRange(2, 1)); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.fars[0].destination.reset(); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.fars[0].destination = upf::Optional<upf::Interface>(upf::N3); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok()); rules = upf_test::basicRules();
  rules.fars[0].action = upf::DROP; rules.fars[0].destination.reset(); rules.fars[0].operation = upf::TUNNEL_NONE;
  REQUIRE(upf::validateRuleSet(rules, limits).ok());
  rules.fars[0].destination = upf::Optional<upf::Interface>(upf::N6); REQUIRE_FALSE(upf::validateRuleSet(rules, limits).ok());
}

TEST_CASE("downlink validation covers required tunnel combinations") {
  upf::RuleSet rules = upf_test::basicRules();
  rules.pdrs[0].sourceInterface = upf::N6; rules.pdrs[0].teid.reset();
  rules.fars[0].destination = upf::Optional<upf::Interface>(upf::N3); rules.fars[0].operation = upf::ADD;
  rules.fars[0].tunnel = upf::Optional<upf::TunnelMetadata>(upf::TunnelMetadata(2001, 9));
  REQUIRE(upf::validateRuleSet(rules, upf::CapacityLimits()).ok());
  rules.pdrs[0].teid = upf::Optional<std::uint32_t>(1); REQUIRE_FALSE(upf::validateRuleSet(rules, upf::CapacityLimits()).ok());
  rules.pdrs[0].teid.reset(); rules.fars[0].tunnel.reset(); REQUIRE_FALSE(upf::validateRuleSet(rules, upf::CapacityLimits()).ok());
}

TEST_CASE("matcher exposes configured mismatches and wildcard matches") {
  upf::RuleSet rules = upf_test::basicRules();
  upf::Packet packet = upf_test::uplinkPacket();
  packet.protocol = upf::TCP; packet.sourcePort = 1; packet.destinationPort = 2;
  upf::PacketPipeline pipeline; REQUIRE(pipeline.install(rules).ok());
  REQUIRE(pipeline.process(packet, "r", "m").terminalResult == upf::NO_MATCH);
  rules.pdrs[0].protocol.reset(); rules.pdrs[0].localPorts.reset(); rules.pdrs[0].remotePorts.reset();
  REQUIRE(pipeline.install(rules).ok());
  REQUIRE(pipeline.process(packet, "r", "m").selectedPdrId == 10);
}

TEST_CASE("FAR validation covers invalid and downlink evaluation branches") {
  upf::Packet packet = upf_test::uplinkPacket();
  upf::Far far; far.id = upf::FarId(1); far.action = upf::DROP;
  REQUIRE(upf::evaluateFar(far, packet).valid);
  far.destination = upf::Optional<upf::Interface>(upf::N6);
  REQUIRE_FALSE(upf::evaluateFar(far, packet).valid);
  far.action = upf::FORWARD; far.destination.reset();
  REQUIRE_FALSE(upf::evaluateFar(far, packet).valid);
  far.destination = upf::Optional<upf::Interface>(upf::N3); far.operation = upf::ADD;
  far.tunnel = upf::Optional<upf::TunnelMetadata>(upf::TunnelMetadata(2, 9));
  REQUIRE_FALSE(upf::evaluateFar(far, packet).valid);
  packet.ingress = upf::N6; packet.tunnel.reset();
  REQUIRE(upf::evaluateFar(far, packet).forwarded);
}

TEST_CASE("QER and URR checked arithmetic covers every overflow path") {
  upf::Qer qer; qer.id = upf::QerId(1); qer.gate = upf::OPEN;
  qer.rateBitsPerSecond = 8000000; qer.burstBytes = 100;
  upf::TokenBucketState state; state.tokensBytes = 0; state.lastRefillMicros = 10;
  REQUIRE(upf::evaluateQer(qer, state, 1, 9).failure == upf::ARITHMETIC_ERROR);
  state.lastRefillMicros = 0; state.remainderBitMicros = 1;
  qer.rateBitsPerSecond = UINT64_MAX;
  REQUIRE(upf::evaluateQer(qer, state, 1, 1).failure == upf::ARITHMETIC_ERROR);
  qer.rateBitsPerSecond = 8000000; state.remainderBitMicros = 0;
  REQUIRE(upf::evaluateQer(qer, state, 1, 200).decision.allowed);
  upf::Urr urr; urr.id = upf::UrrId(1); urr.volumeThresholdBytes = UINT64_MAX;
  upf::UsageCounterState usage; usage.packets = UINT64_MAX;
  REQUIRE(upf::evaluateUrr(urr, usage, 1, "s", 1).overflow);
  usage.packets = 0; usage.payloadBytes = UINT64_MAX;
  REQUIRE(upf::evaluateUrr(urr, usage, 1, "s", 1).overflow);
}

TEST_CASE("scheduler partial and empty cycles preserve kind order") {
  upf::Scheduler scheduler(upf::CapacityLimits(1, 1, 1));
  REQUIRE(scheduler.takeCycle().empty());
  REQUIRE(scheduler.enqueuePacket("p"));
  std::vector<upf::ScheduledItem> packet = scheduler.takeCycle();
  REQUIRE(packet.size() == 1); REQUIRE(packet[0].kind == upf::PACKET_WORK);
  REQUIRE(scheduler.enqueueReconciliation("r"));
  std::vector<upf::ScheduledItem> reconciliation = scheduler.classifyRemainder();
  REQUIRE(reconciliation.size() == 1);
  REQUIRE(reconciliation[0].kind == upf::RECONCILIATION_WORK);
}

TEST_CASE("pipeline covers malformed optional drop closed rate and capacities") {
  upf::PacketPipeline empty; REQUIRE(empty.process(upf_test::uplinkPacket(), "r", "x").terminalResult == upf::NO_MATCH);
  upf::RuleSet rules = upf_test::basicRules(); upf::Packet packet = upf_test::uplinkPacket();
  upf::PacketPipeline pipeline; REQUIRE(pipeline.install(rules).ok());
  packet.sessionId = "other"; REQUIRE(pipeline.process(packet, "r", "x").terminalResult == upf::MALFORMED_PACKET);
  packet = upf_test::uplinkPacket(); packet.tunnel.reset(); REQUIRE(pipeline.process(packet, "r", "x").terminalResult == upf::MALFORMED_PACKET);
  rules = upf_test::basicRules(); rules.pdrs[0].qerId.reset(); rules.pdrs[0].urrId.reset();
  REQUIRE(pipeline.install(rules).ok()); REQUIRE(pipeline.process(upf_test::uplinkPacket(), "r", "x").terminalResult == upf::FORWARDED_TO_N6);
  rules = upf_test::basicRules(); rules.qers[0].gate = upf::CLOSED;
  REQUIRE(pipeline.install(rules).ok()); REQUIRE(pipeline.process(upf_test::uplinkPacket(), "r", "x").terminalResult == upf::GATE_CLOSED);
  rules = upf_test::basicRules(); rules.qers[0].burstBytes = 1; rules.qers[0].generation = 2;
  REQUIRE(pipeline.install(rules).ok()); REQUIRE(pipeline.process(upf_test::uplinkPacket(), "r", "x").terminalResult == upf::RATE_LIMITED);
  rules = upf_test::basicRules(); rules.fars[0].action = upf::DROP; rules.fars[0].destination.reset(); rules.fars[0].operation = upf::TUNNEL_NONE;
  REQUIRE(pipeline.install(rules).ok()); REQUIRE(pipeline.process(upf_test::uplinkPacket(), "r", "x").terminalResult == upf::FAR_DROPPED);
  upf::CapacityLimits noSink; noSink.sinkPackets = 0; upf::PacketPipeline sinkFull(noSink);
  REQUIRE(sinkFull.install(upf_test::basicRules()).ok()); REQUIRE(sinkFull.process(upf_test::uplinkPacket(), "r", "x").terminalResult == upf::CAPACITY_ERROR);
  upf::CapacityLimits noReports; noReports.reports = 0; rules = upf_test::basicRules(); rules.urrs[0].volumeThresholdBytes = 1;
  upf::PacketPipeline reportsFull(noReports); REQUIRE(reportsFull.install(rules).ok());
  REQUIRE(reportsFull.process(upf_test::uplinkPacket(), "r", "x").terminalResult == upf::CAPACITY_ERROR);
}

TEST_CASE("rule service covers empty stale applied and delete transitions") {
  upf::RuleService service; REQUIRE(service.convergence() == upf::EMPTY);
  upf::RuleSet rules = upf_test::basicRules(); rules.version = 1;
  REQUIRE_FALSE(service.update("", rules).ok()); REQUIRE(service.update("u1", rules).ok());
  service.markApplied("wrong", 1); REQUIRE(service.convergence() == upf::DIVERGENT);
  service.markApplied("u1", 1); REQUIRE(service.convergence() == upf::CONVERGED);
  REQUIRE_FALSE(service.remove("", 2).ok()); REQUIRE(service.remove("u2", 2).ok());
  REQUIRE(service.convergence() == upf::DIVERGENT);
}

TEST_CASE("reconciliation covers absent zero delayed and exhausted paths") {
  std::vector<upf::AdapterResult> success(1, upf::AdapterResult(upf::SUCCESS));
  upf::SimulatedDataplane successAdapter(success); upf::RuleService empty;
  REQUIRE_FALSE(upf::ReconciliationController(empty, successAdapter, 1).reconcile().converged);
  upf::RuleService service; upf::RuleSet rules = upf_test::basicRules(); rules.version = 1; REQUIRE(service.update("u", rules).ok());
  REQUIRE_FALSE(upf::ReconciliationController(service, successAdapter, 0).reconcile().converged);
  std::vector<upf::AdapterResult> delayed(1, upf::AdapterResult(upf::DELAYED_SUCCESS, 10));
  upf::SimulatedDataplane delayedAdapter(delayed); REQUIRE(upf::ReconciliationController(service, delayedAdapter, 1).reconcile().delayed);
  std::vector<upf::AdapterResult> failures(2, upf::AdapterResult(upf::TRANSIENT_FAILURE));
  upf::SimulatedDataplane failureAdapter(failures); REQUIRE_FALSE(upf::ReconciliationController(service, failureAdapter, 2).reconcile().converged);
}

TEST_CASE("terminal result names cover every public outcome") {
  const upf::TerminalResult values[] = {upf::FORWARDED_TO_N3, upf::FORWARDED_TO_N6, upf::FAR_DROPPED,
    upf::NO_MATCH, upf::MALFORMED_PACKET, upf::GATE_CLOSED, upf::RATE_LIMITED,
    upf::PACKET_QUEUE_FULL, upf::SHUTDOWN_DEADLINE, upf::ARITHMETIC_ERROR, upf::CAPACITY_ERROR};
  for (std::size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    REQUIRE(upf::terminalResultName(values[i]) != "UNKNOWN");
}
