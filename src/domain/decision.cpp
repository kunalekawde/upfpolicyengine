#include "upf/domain/decision.hpp"

namespace upf {
std::string terminalResultName(TerminalResult result) {
  switch (result) {
  case FORWARDED_TO_N3: return "FORWARDED_TO_N3";
  case FORWARDED_TO_N6: return "FORWARDED_TO_N6";
  case FAR_DROPPED: return "FAR_DROPPED";
  case NO_MATCH: return "NO_MATCH";
  case MALFORMED_PACKET: return "MALFORMED_PACKET";
  case GATE_CLOSED: return "GATE_CLOSED";
  case RATE_LIMITED: return "RATE_LIMITED";
  case PACKET_QUEUE_FULL: return "PACKET_QUEUE_FULL";
  case SHUTDOWN_DEADLINE: return "SHUTDOWN_DEADLINE";
  case ARITHMETIC_ERROR: return "ARITHMETIC_ERROR";
  case CAPACITY_ERROR: return "CAPACITY_ERROR";
  }
  return "UNKNOWN";
}
} // namespace upf
