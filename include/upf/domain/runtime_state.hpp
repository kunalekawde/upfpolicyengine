#pragma once

#include "upf/domain/types.hpp"
#include <map>
#include <string>

namespace upf {

struct TokenBucketState {
  TokenBucketState() : tokensBytes(0), lastRefillMicros(0), remainderBitMicros(0) {}
  std::uint64_t tokensBytes;
  std::uint64_t lastRefillMicros;
  std::uint64_t remainderBitMicros;
};

struct UsageCounterState {
  UsageCounterState() : packets(0), payloadBytes(0), reportEmitted(false) {}
  std::uint64_t packets;
  std::uint64_t payloadBytes;
  bool reportEmitted;
};

struct RuntimeState {
  std::map<std::uint32_t, TokenBucketState> qers;
  std::map<std::uint32_t, UsageCounterState> urrs;
  std::map<std::uint32_t, std::uint32_t> qerGenerations;
  std::map<std::uint32_t, std::uint32_t> urrGenerations;
};

} // namespace upf
