#include "upf/application/scenario_runner.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

namespace upf {
namespace {
Result<nlohmann::json> readJson(const std::string& path) {
  std::ifstream input(path.c_str());
  if (!input) return Result<nlohmann::json>::failure("cannot read " + path);
  try {
    nlohmann::json value;
    input >> value;
    return Result<nlohmann::json>::success(value);
  } catch (const nlohmann::json::exception& error) {
    return Result<nlohmann::json>::failure(error.what());
  }
}
Optional<TransportProtocol> protocol(const nlohmann::json& value) {
  if (!value.is_string()) return Optional<TransportProtocol>();
  const std::string text = value.get<std::string>();
  if (text == "TCP") return Optional<TransportProtocol>(TCP);
  if (text == "UDP") return Optional<TransportProtocol>(UDP);
  if (text == "ICMP") return Optional<TransportProtocol>(ICMP);
  if (text == "ICMPV6") return Optional<TransportProtocol>(ICMPV6);
  return Optional<TransportProtocol>();
}
Result<RuleSet> parseRules(const nlohmann::json& root) {
  try {
    RuleSet rules; rules.sessionId = root.at("sessionId").get<std::string>();
    rules.version = root.at("rulesetVersion").get<std::uint64_t>();
    const nlohmann::json& pdrs = root.at("pdrs");
    for (std::size_t i = 0; i < pdrs.size(); ++i) {
      const nlohmann::json& item = pdrs[i]; Pdr pdr;
      pdr.id = PdrId(item.at("id").get<std::uint32_t>());
      pdr.precedence = item.at("precedence").get<std::uint32_t>();
      pdr.sourceInterface = item.at("sourceInterface") == "N3" ? N3 : N6;
      if (item.contains("teid")) pdr.teid = Optional<std::uint32_t>(item.at("teid").get<std::uint32_t>());
      Result<IpPrefix> prefix = IpPrefix::parse(item.at("uePrefix").get<std::string>());
      if (!prefix.ok()) return Result<RuleSet>::failure(prefix.error());
      pdr.uePrefix = prefix.value();
      if (item.contains("protocol")) pdr.protocol = protocol(item.at("protocol"));
      if (item.contains("localPorts")) pdr.localPorts = Optional<PortRange>(PortRange(item["localPorts"]["first"].get<std::uint16_t>(), item["localPorts"]["last"].get<std::uint16_t>()));
      if (item.contains("remotePorts")) pdr.remotePorts = Optional<PortRange>(PortRange(item["remotePorts"]["first"].get<std::uint16_t>(), item["remotePorts"]["last"].get<std::uint16_t>()));
      if (item.contains("qfi")) pdr.qfi = Optional<std::uint8_t>(item["qfi"].get<std::uint8_t>());
      pdr.farId = FarId(item.at("farId").get<std::uint32_t>());
      if (item.contains("qerId")) pdr.qerId = Optional<QerId>(QerId(item["qerId"].get<std::uint32_t>()));
      if (item.contains("urrId")) pdr.urrId = Optional<UrrId>(UrrId(item["urrId"].get<std::uint32_t>()));
      rules.pdrs.push_back(pdr);
    }
    const nlohmann::json& fars = root.at("fars");
    for (std::size_t i = 0; i < fars.size(); ++i) {
      const nlohmann::json& item = fars[i]; Far far;
      far.id = FarId(item.at("id").get<std::uint32_t>());
      far.action = item.at("action") == "FORWARD" ? FORWARD : DROP;
      if (item.contains("destinationInterface")) far.destination = Optional<Interface>(item["destinationInterface"] == "N3" ? N3 : N6);
      if (item.contains("tunnelOperation")) {
        const std::string op = item["tunnelOperation"].get<std::string>();
        far.operation = op == "ADD" ? ADD : (op == "REMOVE" ? REMOVE : TUNNEL_NONE);
      }
      if (item.contains("tunnel")) far.tunnel = Optional<TunnelMetadata>(TunnelMetadata(item["tunnel"]["teid"].get<std::uint32_t>(), item["tunnel"]["qfi"].get<std::uint8_t>()));
      rules.fars.push_back(far);
    }
    const nlohmann::json& qers = root.at("qers");
    for (std::size_t i = 0; i < qers.size(); ++i) {
      const nlohmann::json& item = qers[i]; Qer qer;
      qer.id = QerId(item.at("id").get<std::uint32_t>()); qer.gate = item.at("gate") == "OPEN" ? OPEN : CLOSED;
      qer.rateBitsPerSecond = item.at("rateBitsPerSecond").get<std::uint64_t>();
      qer.burstBytes = item.at("burstBytes").get<std::uint64_t>();
      qer.generation = item.at("generation").get<std::uint32_t>(); rules.qers.push_back(qer);
    }
    const nlohmann::json& urrs = root.at("urrs");
    for (std::size_t i = 0; i < urrs.size(); ++i) {
      const nlohmann::json& item = urrs[i]; Urr urr;
      urr.id = UrrId(item.at("id").get<std::uint32_t>());
      urr.volumeThresholdBytes = item.at("volumeThresholdBytes").get<std::uint64_t>();
      urr.generation = item.at("generation").get<std::uint32_t>(); rules.urrs.push_back(urr);
    }
    return Result<RuleSet>::success(rules);
  } catch (const nlohmann::json::exception& error) {
    return Result<RuleSet>::failure(error.what());
  }
}
Result<std::vector<Packet> > parsePackets(const nlohmann::json& root) {
  try {
    std::vector<Packet> packets;
    for (std::size_t i = 0; i < root.at("packets").size(); ++i) {
      const nlohmann::json& item = root["packets"][i]; Packet packet;
      packet.packetId = item.at("packetId").get<std::string>(); packet.sessionId = item.at("sessionId").get<std::string>();
      packet.ingress = item.at("ingress") == "N3" ? N3 : N6;
      if (item.contains("tunnel")) packet.tunnel = Optional<TunnelMetadata>(TunnelMetadata(item["tunnel"]["teid"].get<std::uint32_t>(), item["tunnel"]["qfi"].get<std::uint8_t>()));
      Result<IpAddress> source = IpAddress::parse(item.at("sourceIp").get<std::string>());
      Result<IpAddress> destination = IpAddress::parse(item.at("destinationIp").get<std::string>());
      if (!source.ok() || !destination.ok()) return Result<std::vector<Packet> >::failure("invalid packet address");
      packet.sourceIp = source.value(); packet.destinationIp = destination.value();
      Optional<TransportProtocol> p = protocol(item.at("protocol"));
      if (!p.hasValue()) return Result<std::vector<Packet> >::failure("invalid protocol");
      packet.protocol = p.value(); packet.sourcePort = item.at("sourcePort").get<std::uint16_t>();
      packet.destinationPort = item.at("destinationPort").get<std::uint16_t>();
      packet.payloadBytes = item.at("payloadBytes").get<std::uint32_t>();
      packet.arrivalTimeMicros = item.at("arrivalTimeMicros").get<std::uint64_t>(); packets.push_back(packet);
    }
    return Result<std::vector<Packet> >::success(packets);
  } catch (const nlohmann::json::exception& error) {
    return Result<std::vector<Packet> >::failure(error.what());
  }
}
} // namespace

Result<RunResult> runFiles(const std::string& rulesPath, const std::string& packetsPath) {
  Result<nlohmann::json> rulesJson = readJson(rulesPath);
  Result<nlohmann::json> packetsJson = readJson(packetsPath);
  if (!rulesJson.ok()) return Result<RunResult>::failure(rulesJson.error());
  if (!packetsJson.ok()) return Result<RunResult>::failure(packetsJson.error());
  Result<RuleSet> rules = parseRules(rulesJson.value());
  Result<std::vector<Packet> > packets = parsePackets(packetsJson.value());
  if (!rules.ok()) return Result<RunResult>::failure(rules.error());
  if (!packets.ok()) return Result<RunResult>::failure(packets.error());
  PacketPipeline pipeline;
  Result<void> installed = pipeline.install(rules.value());
  if (!installed.ok()) return Result<RunResult>::failure(installed.error());
  RunResult result; result.runId = packetsJson.value().value("runId", "file-run");
  result.scenarioId = packetsJson.value().value("scenarioId", "local-files");
  for (std::size_t i = 0; i < packets.value().size(); ++i)
    result.decisions.push_back(pipeline.process(packets.value()[i], result.runId, result.scenarioId));
  result.n3Sink = pipeline.n3Sink(); result.n6Sink = pipeline.n6Sink();
  result.intendedVersion = rules.value().version; result.appliedVersion = rules.value().version;
  result.activeVersion = rules.value().version; result.convergence = CONVERGED;
  return Result<RunResult>::success(result);
}

} // namespace upf
