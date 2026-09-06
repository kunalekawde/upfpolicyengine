#include <catch2/catch.hpp>
#include "upf/domain/address.hpp"

TEST_CASE("IPv4 prefix boundaries hold for every host in a small subnet") {
  const upf::IpPrefix prefix = upf::IpPrefix::parse("192.0.2.0/24").value();
  for (unsigned int host = 0; host <= 255; ++host) {
    const std::string text = "192.0.2." + std::to_string(host);
    REQUIRE(prefix.contains(upf::IpAddress::parse(text).value()));
  }
  REQUIRE_FALSE(prefix.contains(upf::IpAddress::parse("192.0.3.0").value()));
}
