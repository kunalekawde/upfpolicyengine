#include <catch2/catch.hpp>
#include "tests/support/test_builders.hpp"
#include "upf/application/reconciliation.hpp"

TEST_CASE("transient failures retry with stable identity and converge") {
  upf::RuleService service; upf::RuleSet rules = upf_test::basicRules(); rules.version = 1;
  REQUIRE(service.update("stable", rules).ok());
  std::vector<upf::AdapterResult> outcomes;
  outcomes.push_back(upf::AdapterResult(upf::TRANSIENT_FAILURE));
  outcomes.push_back(upf::AdapterResult(upf::SUCCESS));
  upf::SimulatedDataplane adapter(outcomes);
  upf::ReconciliationController controller(service, adapter, 3);
  upf::ReconciliationStatus status = controller.reconcile();
  REQUIRE(status.converged); REQUIRE(status.attempts == 2);
  REQUIRE(service.appliedVersion() == 1);
}

TEST_CASE("permanent failure remains divergent") {
  upf::RuleService service; upf::RuleSet rules = upf_test::basicRules(); rules.version = 1;
  REQUIRE(service.update("u", rules).ok());
  std::vector<upf::AdapterResult> outcomes(1, upf::AdapterResult(upf::PERMANENT_FAILURE));
  upf::SimulatedDataplane adapter(outcomes); upf::ReconciliationController controller(service, adapter, 3);
  REQUIRE_FALSE(controller.reconcile().converged);
  REQUIRE(service.convergence() == upf::DIVERGENT);
}
