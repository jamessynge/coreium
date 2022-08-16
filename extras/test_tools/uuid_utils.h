#ifndef MCUCORE_EXTRAS_TEST_TOOLS_UUID_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_UUID_UTILS_H_

// Test helpers for working with UUID values.

#include "gmock/gmock.h"
#include "misc/uuid.h"

namespace mcucore {
namespace test {

const char kUuidRegex[] = R"re([0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-)re"
                          R"re([0-9A-Fa-f]{4}-[0-9A-Fa-f]{12})re";

MATCHER(JsonValueIsUuid, "") {
  static auto matcher = testing::MatchesRegex(kUuidRegex);  // NOLINT
  if (!arg.is_string()) {
    return false;
  }
  if (!testing::Matches(matcher)(arg.as_string())) {
    return false;
  }
  return true;
}

template <int N>
Uuid MakeUuid(const uint8_t (&data)[N]) {
  Uuid uuid;
  uuid.SetForTest(data);
  return uuid;
}

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_UUID_UTILS_H_
