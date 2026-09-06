#include "upf/domain/address.hpp"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace upf {

IpAddress::IpAddress() : family_(IPV4), bytes_() {}

Result<IpAddress> IpAddress::parse(const std::string& text) {
  IpAddress address;
  if (inet_pton(AF_INET, text.c_str(), address.bytes_.data()) == 1) {
    address.family_ = IPV4;
    return Result<IpAddress>::success(address);
  }
  address.bytes_.fill(0);
  if (inet_pton(AF_INET6, text.c_str(), address.bytes_.data()) == 1) {
    address.family_ = IPV6;
    return Result<IpAddress>::success(address);
  }
  return Result<IpAddress>::failure("invalid IP address");
}

std::string IpAddress::toString() const {
  char buffer[INET6_ADDRSTRLEN] = {};
  const int family = family_ == IPV4 ? AF_INET : AF_INET6;
  if (inet_ntop(family, bytes_.data(), buffer, sizeof(buffer)) == 0) return "";
  return buffer;
}

bool IpAddress::operator==(const IpAddress& other) const {
  if (family_ != other.family_) return false;
  const std::size_t size = family_ == IPV4 ? 4U : 16U;
  return std::memcmp(bytes_.data(), other.bytes_.data(), size) == 0;
}

IpPrefix::IpPrefix() : network_(), length_(0) {}

Result<IpPrefix> IpPrefix::parse(const std::string& text) {
  const std::size_t slash = text.find('/');
  if (slash == std::string::npos) return Result<IpPrefix>::failure("prefix length missing");
  Result<IpAddress> parsed = IpAddress::parse(text.substr(0, slash));
  if (!parsed.ok()) return Result<IpPrefix>::failure(parsed.error());
  char* end = 0;
  const std::string lengthText = text.substr(slash + 1);
  const long length = std::strtol(lengthText.c_str(), &end, 10);
  const long maximum = parsed.value().family() == IPV4 ? 32 : 128;
  if (end == 0 || *end != '\0' || length < 0 || length > maximum)
    return Result<IpPrefix>::failure("invalid prefix length");
  IpPrefix prefix;
  prefix.network_ = parsed.value();
  prefix.length_ = static_cast<std::uint8_t>(length);
  const std::array<std::uint8_t, 16>& bytes = prefix.network_.bytes();
  for (std::size_t bit = static_cast<std::size_t>(length); bit < static_cast<std::size_t>(maximum); ++bit) {
    if ((bytes[bit / 8U] & static_cast<std::uint8_t>(0x80U >> (bit % 8U))) != 0U)
      return Result<IpPrefix>::failure("prefix contains host bits");
  }
  return Result<IpPrefix>::success(prefix);
}

bool IpPrefix::contains(const IpAddress& address) const {
  if (network_.family() != address.family()) return false;
  const std::array<std::uint8_t, 16>& left = network_.bytes();
  const std::array<std::uint8_t, 16>& right = address.bytes();
  const std::size_t whole = length_ / 8U;
  const std::size_t rest = length_ % 8U;
  if (std::memcmp(left.data(), right.data(), whole) != 0) return false;
  if (rest == 0U) return true;
  const std::uint8_t mask = static_cast<std::uint8_t>(0xFFU << (8U - rest));
  return (left[whole] & mask) == (right[whole] & mask);
}

} // namespace upf
