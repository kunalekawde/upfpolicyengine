#include "upf/application/scenario_runner.hpp"
#include <iostream>

namespace {
const char* disclaimer =
    "Simulation-only educational demo; not a conformant or production PFCP, GTP-U, or UPF implementation.";

int usage() {
  std::cerr << "Usage: upf-rule-pipeline-demo --scenario NAME [--output human|json]\n"
            << "       upf-rule-pipeline-demo --rules FILE --packets FILE [--output human|json]\n";
  return 2;
}
}

int main(int argc, char** argv) {
  std::string scenario;
  std::string rules;
  std::string packets;
  std::string output = "human";
  bool list = false;
  for (int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if (arg == "--help") {
      std::cout << disclaimer << "\nUsage: upf-rule-pipeline-demo --scenario NAME [--output human|json]\n";
      return 0;
    }
    if (arg == "--list-scenarios") { list = true; continue; }
    if ((arg == "--scenario" || arg == "--rules" || arg == "--packets" || arg == "--output") && i + 1 < argc) {
      const std::string value(argv[++i]);
      if (arg == "--scenario") scenario = value;
      else if (arg == "--rules") rules = value;
      else if (arg == "--packets") packets = value;
      else output = value;
      continue;
    }
    return usage();
  }
  if (list) {
    const std::vector<std::string> names = upf::ScenarioRunner::scenarioNames();
    for (std::size_t i = 0; i < names.size(); ++i) std::cout << names[i] << "\n";
    std::cout << disclaimer << "\n";
    return 0;
  }
  if (output != "human" && output != "json") return usage();
  upf::RunResult result;
  if (!scenario.empty() && rules.empty() && packets.empty()) {
    const std::vector<std::string> names = upf::ScenarioRunner::scenarioNames();
    bool known = false;
    for (std::size_t i = 0; i < names.size(); ++i) known = known || names[i] == scenario;
    if (!known) return usage();
    result = upf::ScenarioRunner().run(scenario);
  } else if (scenario.empty() && !rules.empty() && !packets.empty()) {
    upf::Result<upf::RunResult> loaded = upf::runFiles(rules, packets);
    if (!loaded.ok()) { std::cerr << loaded.error() << "\n"; return 4; }
    result = loaded.value();
  } else {
    return usage();
  }
  std::cout << (output == "json" ? upf::renderJson(result) : upf::renderHuman(result));
  if (output == "json") std::cout << "\n";
  return 0;
}
