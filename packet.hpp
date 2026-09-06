#pragma once

#include "upf/domain/address.hpp"

namespace upf {

struct TunnelMetadata {
  TunnelMetadata(std::uint32_t teidValue = 0, std::uint8_t qfiValue = 0)
      : teid(teidValue), qfi(qfiValue) {}
  bool valid() const { return teid != 0 && qfi >= 1 && qfi <= 63; }
  bool operator==(const TunnelMetadata& other) const {
    return teid == other.teid && qfi == other.qfi;
  }
  std::uint32_t teid;
  std::uint8_t qfi;
};

struct Packet {
  std::string packetId;
  std::string sessionId;
  Interface ingress;
  Optional<TunnelMetadata> tunnel;
  IpAddress sourceIp;
  IpAddress destinationIp;
  TransportProtocol protocol;
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  std::uint32_t payloadBytes;
  std::uint64_t arrivalTimeMicros;
};

} // namespace upf
