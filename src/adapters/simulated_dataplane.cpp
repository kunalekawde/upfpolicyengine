#include "upf/ports/dataplane_adapter.hpp"
#include <sstream>

namespace upf {

SimulatedDataplane::SimulatedDataplane(const std::vector<AdapterResult>& outcomes)
    : outcomes_(outcomes), next_(0) {}

std::string SimulatedDataplane::key(const std::string& updateId, std::uint64_t version) const {
  std::ostringstream out; out << updateId << ':' << version; return out.str();
}

AdapterResult SimulatedDataplane::apply(const std::string& updateId, std::uint64_t version,
                                        std::uint32_t attempt) {
  (void)attempt;
  const std::string identity = key(updateId, version);
  if (applied_.count(identity) != 0U) return AdapterResult(SUCCESS);
  const AdapterResult result = next_ < outcomes_.size() ? outcomes_[next_++] : AdapterResult(SUCCESS);
  if (result.outcome == SUCCESS) applied_.insert(identity);
  if (result.outcome == DELAYED_SUCCESS) delayed_.insert(identity);
  return result;
}

bool SimulatedDataplane::completeDelayed(const std::string& updateId, std::uint64_t version,
                                         const std::string& latestUpdateId,
                                         std::uint64_t latestVersion) {
  const std::string identity = key(updateId, version);
  if (applied_.count(identity) != 0U) return true;
  if (updateId != latestUpdateId || version != latestVersion || delayed_.count(identity) == 0U)
    return false;
  applied_.insert(identity);
  delayed_.erase(identity);
  return true;
}

} // namespace upf
