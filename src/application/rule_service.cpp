#include "upf/application/rule_service.hpp"

namespace upf {

RuleService::RuleService(const CapacityLimits& limits) : limits_(limits), appliedVersion_(0) {}

Result<void> RuleService::update(const std::string& updateId, const RuleSet& candidate) {
  if (updateId.empty()) return Result<void>::failure("update ID required");
  if (active_ && candidate.version <= active_->version)
    return Result<void>::failure("ruleset version must increase");
  Result<void> valid = validateRuleSet(candidate, limits_);
  if (!valid.ok()) return valid;
  RuntimeState migrated;
  for (std::size_t i = 0; i < candidate.qers.size(); ++i) {
    const Qer& qer = candidate.qers[i];
    std::map<std::uint32_t, std::uint32_t>::const_iterator generation =
        runtime_.qerGenerations.find(qer.id.value);
    if (generation != runtime_.qerGenerations.end() && generation->second == qer.generation)
      migrated.qers[qer.id.value] = runtime_.qers[qer.id.value];
    else {
      TokenBucketState state;
      state.tokensBytes = qer.burstBytes;
      migrated.qers[qer.id.value] = state;
    }
    migrated.qerGenerations[qer.id.value] = qer.generation;
  }
  for (std::size_t i = 0; i < candidate.urrs.size(); ++i) {
    const Urr& urr = candidate.urrs[i];
    std::map<std::uint32_t, std::uint32_t>::const_iterator generation =
        runtime_.urrGenerations.find(urr.id.value);
    if (generation != runtime_.urrGenerations.end() && generation->second == urr.generation)
      migrated.urrs[urr.id.value] = runtime_.urrs[urr.id.value];
    else
      migrated.urrs[urr.id.value] = UsageCounterState();
    migrated.urrGenerations[urr.id.value] = urr.generation;
  }
  std::shared_ptr<const RuleSet> snapshot(new RuleSet(candidate));
  runtime_ = migrated;
  intended_ = snapshot;
  active_ = snapshot;
  latestUpdateId_ = updateId;
  return Result<void>::success();
}

Result<void> RuleService::remove(const std::string& updateId, std::uint64_t version) {
  if (!active_ || updateId.empty() || version <= active_->version)
    return Result<void>::failure("invalid delete");
  intended_.reset();
  active_.reset();
  latestUpdateId_ = updateId;
  return Result<void>::success();
}

ConvergenceStatus RuleService::convergence() const {
  if (!intended_ && appliedVersion_ == 0) return EMPTY;
  if (intended_ && intended_->version == appliedVersion_) return CONVERGED;
  return DIVERGENT;
}

void RuleService::markApplied(const std::string& updateId, std::uint64_t version) {
  if (intended_ && updateId == latestUpdateId_ && version == intended_->version)
    appliedVersion_ = version;
}

} // namespace upf
