#include <catch2/catch.hpp>
#include "upf/domain/address.hpp"
#include "upf/domain/types.hpp"

TEST_CASE("checked arithmetic rejects overflow") {
  std::uint64_t out = 0;
  REQUIRE(upf::checkedAdd(4, 5, out));
  REQUIRE(out == 9);
  REQUIRE_FALSE(upf::checkedAdd(UINT64_MAX, 1, out));
}

TEST_CASE("address prefix and inclusive ports are deterministic") {
  upf::Result<upf::IpPrefix> prefix = upf::IpPrefix::parse("10.10.0.0/24");
  REQUIRE(prefix.ok());
  REQUIRE(prefix.value().contains(upf::IpAddress::parse("10.10.0.0").value()));
  REQUIRE(prefix.value().contains(upf::IpAddress::parse("10.10.0.255").value()));
  REQUIRE_FALSE(prefix.value().contains(upf::IpAddress::parse("10.10.1.0").value()));
  REQUIRE(upf::PortRange(443, 443).contains(443));
  REQUIRE_FALSE(upf::PortRange(443, 443).contains(442));
}

TEST_CASE("invalid and family-mismatched addresses fail closed") {
  REQUIRE_FALSE(upf::IpAddress::parse("999.1.1.1").ok());
  upf::Result<upf::IpPrefix> v6 = upf::IpPrefix::parse("2001:db8::/32");
  REQUIRE(v6.ok());
  REQUIRE_FALSE(v6.value().contains(upf::IpAddress::parse("192.0.2.1").value()));
}
