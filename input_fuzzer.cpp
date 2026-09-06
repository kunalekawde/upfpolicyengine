#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  const nlohmann::json value = nlohmann::json::parse(data, data + size, 0, false);
  if (value.is_discarded()) return 0;
  if (value.is_object()) {
    volatile std::size_t fields = value.size();
    (void)fields;
  }
  return 0;
}
