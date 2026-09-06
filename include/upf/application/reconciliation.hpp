#pragma once

#include "upf/application/rule_service.hpp"
#include "upf/ports/dataplane_adapter.hpp"

namespace upf {

struct ReconciliationStatus {
  ReconciliationStatus() : converged(false), attempts(0), delayed(false) {}
  bool converged;
  std::uint32_t attempts;
  bool delayed;
};

class ReconciliationController {
public:
  ReconciliationController(RuleService& service, DataplaneAdapter& adapter,
                           std::uint32_t maximumAttempts)
      : service_(service), adapter_(adapter), maximumAttempts_(maximumAttempts) {}
  ReconciliationStatus reconcile();
private:
  RuleService& service_;
  DataplaneAdapter& adapter_;
  std::uint32_t maximumAttempts_;
};

} // namespace upf
