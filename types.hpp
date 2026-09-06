#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

namespace upf {

template <typename T> class Optional {
public:
  Optional() : present_(false), value_() {}
  explicit Optional(const T& value) : present_(true), value_(value) {}
  bool hasValue() const { return present_; }
  const T& value() const {
    if (!present_) throw std::logic_error("optional has no value");
    return value_;
  }
  T& value() {
    if (!present_) throw std::logic_error("optional has no value");
    return value_;
  }
  void reset() { present_ = false; value_ = T(); }
  bool operator==(const Optional<T>& other) const {
    return present_ == other.present_ && (!present_ || value_ == other.value_);
  }
private:
  bool present_;
  T value_;
};

template <typename T> class Result {
public:
  static Result success(const T& value) { return Result(true, value, ""); }
  static Result failure(const std::string& error) { return Result(false, T(), error); }
  bool ok() const { return ok_; }
  const T& value() const {
    if (!ok_) throw std::logic_error(error_);
    return value_;
  }
  T& value() {
    if (!ok_) throw std::logic_error(error_);
    return value_;
  }
  const std::string& error() const { return error_; }
private:
  Result(bool ok, const T& value, const std::string& error)
      : ok_(ok), value_(value), error_(error) {}
  bool ok_;
  T value_;
  std::string error_;
};

template <> class Result<void> {
public:
  static Result success() { return Result(true, ""); }
  static Result failure(const std::string& error) { return Result(false, error); }
  bool ok() const { return ok_; }
  const std::string& error() const { return error_; }
private:
  Result(bool ok, const std::string& error) : ok_(ok), error_(error) {}
  bool ok_;
  std::string error_;
};

inline bool checkedAdd(std::uint64_t a, std::uint64_t b, std::uint64_t& out) {
  if (b > std::numeric_limits<std::uint64_t>::max() - a) return false;
  out = a + b;
  return true;
}

inline bool checkedMultiply(std::uint64_t a, std::uint64_t b, std::uint64_t& out) {
  if (a != 0 && b > std::numeric_limits<std::uint64_t>::max() / a) return false;
  out = a * b;
  return true;
}

struct RuleId {
  explicit RuleId(std::uint32_t v = 0) : value(v) {}
  bool valid() const { return value != 0; }
  bool operator==(const RuleId& other) const { return value == other.value; }
  bool operator<(const RuleId& other) const { return value < other.value; }
  std::uint32_t value;
};

typedef RuleId PdrId;
typedef RuleId FarId;
typedef RuleId QerId;
typedef RuleId UrrId;

enum Interface { N3, N6 };
enum TransportProtocol { TCP, UDP, ICMP, ICMPV6 };
enum GateStatus { OPEN, CLOSED };
enum FarAction { FORWARD, DROP };
enum TunnelOperation { TUNNEL_NONE, ADD, REMOVE };
enum AdapterOutcome { SUCCESS, DELAYED_SUCCESS, TRANSIENT_FAILURE, PERMANENT_FAILURE };
enum ConvergenceStatus { EMPTY, CONVERGED, DIVERGENT };

struct CapacityLimits {
  CapacityLimits(std::size_t control = 128, std::size_t packet = 1024,
                 std::size_t reconciliation = 128)
      : controlQueue(control), packetQueue(packet), reconciliationQueue(reconciliation),
        reports(1024), results(2048), events(8192), sinkPackets(2048), sessions(16),
        rulesPerType(256) {}
  std::size_t controlQueue;
  std::size_t packetQueue;
  std::size_t reconciliationQueue;
  std::size_t reports;
  std::size_t results;
  std::size_t events;
  std::size_t sinkPackets;
  std::size_t sessions;
  std::size_t rulesPerType;
};

} // namespace upf
