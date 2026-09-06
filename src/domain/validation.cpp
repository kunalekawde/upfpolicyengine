#include "upf/domain/rules.hpp"
#include <set>

namespace upf {

const Far* findFar(const RuleSet& rules, FarId id) {
  for (std::size_t i = 0; i < rules.fars.size(); ++i)
    if (rules.fars[i].id == id) return &rules.fars[i];
  return 0;
}
const Qer* findQer(const RuleSet& rules, QerId id) {
  for (std::size_t i = 0; i < rules.qers.size(); ++i)
    if (rules.qers[i].id == id) return &rules.qers[i];
  return 0;
}
const Urr* findUrr(const RuleSet& rules, UrrId id) {
  for (std::size_t i = 0; i < rules.urrs.size(); ++i)
    if (rules.urrs[i].id == id) return &rules.urrs[i];
  return 0;
}

template <typename T> static bool uniqueIds(const std::vector<T>& values) {
  std::set<std::uint32_t> ids;
  for (std::size_t i = 0; i < values.size(); ++i)
    if (!values[i].id.valid() || !ids.insert(values[i].id.value).second) return false;
  return true;
}

static bool rangesOverlap(const Optional<PortRange>& a, const Optional<PortRange>& b) {
  return !a.hasValue() || !b.hasValue() ||
      (a.value().first <= b.value().last && b.value().first <= a.value().last);
}

template <typename T> static bool optionalValuesOverlap(const Optional<T>& a, const Optional<T>& b) {
  return !a.hasValue() || !b.hasValue() || a.value() == b.value();
}

static bool matchSpacesOverlap(const Pdr& a, const Pdr& b) {
  return a.sourceInterface == b.sourceInterface &&
      optionalValuesOverlap(a.teid, b.teid) &&
      (a.uePrefix.contains(b.uePrefix.network()) || b.uePrefix.contains(a.uePrefix.network())) &&
      optionalValuesOverlap(a.protocol, b.protocol) && rangesOverlap(a.localPorts, b.localPorts) &&
      rangesOverlap(a.remotePorts, b.remotePorts) && optionalValuesOverlap(a.qfi, b.qfi);
}

Result<void> validateRuleSet(const RuleSet& rules, const CapacityLimits& limits) {
  if (rules.sessionId.empty() || rules.version == 0) return Result<void>::failure("invalid session/version");
  if (rules.pdrs.size() > limits.rulesPerType || rules.fars.size() > limits.rulesPerType ||
      rules.qers.size() > limits.rulesPerType || rules.urrs.size() > limits.rulesPerType)
    return Result<void>::failure("rule capacity exceeded");
  if (!uniqueIds(rules.pdrs) || !uniqueIds(rules.fars) || !uniqueIds(rules.qers) ||
      !uniqueIds(rules.urrs)) return Result<void>::failure("invalid or duplicate rule ID");
  for (std::size_t i = 0; i < rules.pdrs.size(); ++i) {
    const Pdr& pdr = rules.pdrs[i];
    const Far* far = findFar(rules, pdr.farId);
    if (far == 0) return Result<void>::failure("missing FAR");
    if (pdr.qerId.hasValue() && findQer(rules, pdr.qerId.value()) == 0)
      return Result<void>::failure("missing QER");
    if (pdr.urrId.hasValue() && findUrr(rules, pdr.urrId.value()) == 0)
      return Result<void>::failure("missing URR");
    if (pdr.sourceInterface == N3 && !pdr.teid.hasValue())
      return Result<void>::failure("N3 PDR requires TEID");
    if (pdr.sourceInterface == N6 && pdr.teid.hasValue())
      return Result<void>::failure("N6 PDR forbids TEID");
    if (pdr.localPorts.hasValue() && !pdr.localPorts.value().valid())
      return Result<void>::failure("invalid local ports");
    if (pdr.remotePorts.hasValue() && !pdr.remotePorts.value().valid())
      return Result<void>::failure("invalid remote ports");
    if (far->action == FORWARD) {
      if (!far->destination.hasValue()) return Result<void>::failure("forward destination missing");
      if (pdr.sourceInterface == N3 && (far->destination.value() != N6 || far->operation != REMOVE))
        return Result<void>::failure("invalid uplink FAR");
      if (pdr.sourceInterface == N6 && (far->destination.value() != N3 || far->operation != ADD ||
          !far->tunnel.hasValue() || !far->tunnel.value().valid()))
        return Result<void>::failure("invalid downlink FAR");
    } else if (far->destination.hasValue() || far->operation != TUNNEL_NONE) {
      return Result<void>::failure("drop FAR has forwarding parameters");
    }
    for (std::size_t j = i + 1; j < rules.pdrs.size(); ++j) {
      if (pdr.precedence == rules.pdrs[j].precedence && matchSpacesOverlap(pdr, rules.pdrs[j]))
        return Result<void>::failure("equal-precedence PDR match spaces overlap");
    }
  }
  return Result<void>::success();
}

} // namespace upf
