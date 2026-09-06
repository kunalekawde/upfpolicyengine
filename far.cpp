#include "upf/domain/engine.hpp"

namespace upf {

FarEvaluation evaluateFar(const Far& far, const Packet& packet) {
  FarEvaluation result;
  result.packet = packet;
  if (far.action == DROP) {
    result.valid = !far.destination.hasValue() && far.operation == TUNNEL_NONE;
    return result;
  }
  if (!far.destination.hasValue()) return result;
  result.destination = far.destination.value();
  if (packet.ingress == N3 && result.destination == N6 && far.operation == REMOVE) {
    result.packet.tunnel.reset();
    result.valid = true;
    result.forwarded = true;
  } else if (packet.ingress == N6 && result.destination == N3 && far.operation == ADD &&
             far.tunnel.hasValue()) {
    result.packet.tunnel = far.tunnel;
    result.valid = true;
    result.forwarded = true;
  }
  return result;
}

} // namespace upf
