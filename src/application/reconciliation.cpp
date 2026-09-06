#include "upf/application/reconciliation.hpp"

namespace upf {

ReconciliationStatus ReconciliationController::reconcile() {
  ReconciliationStatus status;
  std::shared_ptr<const RuleSet> intended = service_.intended();
  if (!intended || maximumAttempts_ == 0) return status;
  for (std::uint32_t attempt = 1; attempt <= maximumAttempts_; ++attempt) {
    ++status.attempts;
    const AdapterResult result = adapter_.apply(service_.latestUpdateId(), intended->version, attempt);
    if (result.outcome == SUCCESS) {
      service_.markApplied(service_.latestUpdateId(), intended->version);
      status.converged = true;
      return status;
    }
    if (result.outcome == DELAYED_SUCCESS) {
      status.delayed = true;
      return status;
    }
    if (result.outcome == PERMANENT_FAILURE) return status;
  }
  return status;
}

} // namespace upf
