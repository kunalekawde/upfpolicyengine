#pragma once
#include "upf/domain/types.hpp"
#include <cstdint>
#include <set>
#include <string>
#include <vector>
namespace upf {
struct AdapterResult {
  AdapterResult(AdapterOutcome value = SUCCESS, std::uint64_t due = 0) : outcome(value), dueTimeMicros(due) {}
  AdapterOutcome outcome;
  std::uint64_t dueTimeMicros;
};
class DataplaneAdapter {
public:
  virtual ~DataplaneAdapter() {}
  virtual AdapterResult apply(const std::string& updateId, std::uint64_t version,
                              std::uint32_t attempt) = 0;
};

class SimulatedDataplane : public DataplaneAdapter {
public:
  explicit SimulatedDataplane(const std::vector<AdapterResult>& outcomes);
  AdapterResult apply(const std::string& updateId, std::uint64_t version,
                      std::uint32_t attempt);
  bool completeDelayed(const std::string& updateId, std::uint64_t version,
                       const std::string& latestUpdateId, std::uint64_t latestVersion);
private:
  std::string key(const std::string& updateId, std::uint64_t version) const;
  std::vector<AdapterResult> outcomes_;
  std::size_t next_;
  std::set<std::string> applied_;
  std::set<std::string> delayed_;
};
} // namespace upf
