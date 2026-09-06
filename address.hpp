#pragma once

#include "upf/domain/types.hpp"
#include <array>
#include <cstdint>
#include <string>

namespace upf {

enum AddressFamily { IPV4, IPV6 };

class IpAddress {
public:
  IpAddress();
  static Result<IpAddress> parse(const std::string& text);
  AddressFamily family() const { return family_; }
  const std::array<std::uint8_t, 16>& bytes() const { return bytes_; }
  std::string toString() const;
  bool operator==(const IpAddress& other) const;
private:
  AddressFamily family_;
  std::array<std::uint8_t, 16> bytes_;
};

class IpPrefix {
public:
  IpPrefix();
  static Result<IpPrefix> parse(const std::string& text);
  bool contains(const IpAddress& address) const;
  const IpAddress& network() const { return network_; }
  std::uint8_t length() const { return length_; }
private:
  IpAddress network_;
  std::uint8_t length_;
};

struct PortRange {
  PortRange(std::uint16_t firstValue = 0, std::uint16_t lastValue = 65535)
      : first(firstValue), last(lastValue) {}
  bool valid() const { return first <= last; }
  bool contains(std::uint16_t port) const { return valid() && port >= first && port <= last; }
  std::uint16_t first;
  std::uint16_t last;
};

} // namespace upf
