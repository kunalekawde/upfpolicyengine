#pragma once

#include "upf/domain/rules.hpp"
#include "upf/domain/runtime_state.hpp"
#include <memory>
#include <string>

namespace upf {

class RuleService {
public:
  explicit RuleService(const CapacityLimits& limits = CapacityLimits());
  Result<void> update(const std::string& updateId, const RuleSet& candidate);
  Result<void> remove(const std::string& updateId, std::uint64_t version);
  std::shared_ptr<const RuleSet> active() const { return active_; }
  std::shared_ptr<const RuleSet> intended() const { return intended_; }
  std::uint64_t appliedVersion() const { return appliedVersion_; }
  ConvergenceStatus convergence() const;
  void markApplied(const std::string& updateId, std::uint64_t version);
  const std::string& latestUpdateId() const { return latestUpdateId_; }
  RuntimeState& runtime() { return runtime_; }
  const RuntimeState& runtime() const { return runtime_; }
private:
  CapacityLimits limits_;
  std::shared_ptr<const RuleSet> intended_;
  std::shared_ptr<const RuleSet> active_;
  std::uint64_t appliedVersion_;
  std::string latestUpdateId_;
  RuntimeState runtime_;
};

} // namespace upf
