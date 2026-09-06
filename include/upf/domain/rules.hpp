#pragma once

#include "upf/domain/packet.hpp"
#include <map>
#include <vector>

namespace upf {

struct Pdr {
  Pdr() : id(), precedence(0), sourceInterface(N3), farId(), generation(1) {}
  PdrId id;
  std::uint32_t precedence;
  Interface sourceInterface;
  Optional<std::uint32_t> teid;
  IpPrefix uePrefix;
  Optional<TransportProtocol> protocol;
  Optional<PortRange> localPorts;
  Optional<PortRange> remotePorts;
  Optional<std::uint8_t> qfi;
  FarId farId;
  Optional<QerId> qerId;
  Optional<UrrId> urrId;
  std::uint32_t generation;
};

struct Far {
  Far() : id(), action(DROP), operation(TUNNEL_NONE) {}
  FarId id;
  FarAction action;
  Optional<Interface> destination;
  TunnelOperation operation;
  Optional<TunnelMetadata> tunnel;
};

struct Qer {
  Qer() : id(), gate(OPEN), rateBitsPerSecond(0), burstBytes(0), generation(1) {}
  QerId id;
  GateStatus gate;
  std::uint64_t rateBitsPerSecond;
  std::uint64_t burstBytes;
  Optional<std::uint8_t> qfi;
  std::uint32_t generation;
};

struct Urr {
  Urr() : id(), volumeThresholdBytes(1), generation(1) {}
  UrrId id;
  std::uint64_t volumeThresholdBytes;
  std::uint32_t generation;
};

struct RuleSet {
  RuleSet() : version(1) {}
  std::string sessionId;
  std::uint64_t version;
  std::vector<Pdr> pdrs;
  std::vector<Far> fars;
  std::vector<Qer> qers;
  std::vector<Urr> urrs;
};

const Far* findFar(const RuleSet& rules, FarId id);
const Qer* findQer(const RuleSet& rules, QerId id);
const Urr* findUrr(const RuleSet& rules, UrrId id);
Result<void> validateRuleSet(const RuleSet& rules, const CapacityLimits& limits);

} // namespace upf
