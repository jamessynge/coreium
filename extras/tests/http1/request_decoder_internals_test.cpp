// Tests of helper functions in mcucore::http1::mcucore_http1_internal.

#include "gtest/gtest.h"
#include "http1/request_decoder.h"

namespace mcucore {
namespace http1 {
namespace mcucore_http1_internal {
namespace test {
namespace {

using ::mcucore::StringView;

TEST(RequestDecoderInternalsTest, IsPChar) {
  EXPECT_FALSE(IsPChar(' '));
  auto* test = IsPChar;
  EXPECT_FALSE(test(' '));
}

TEST(RequestDecoderInternalsTest, IsQueryChar) {
  EXPECT_FALSE(IsQueryChar(' '));
  auto* test = IsQueryChar;
  EXPECT_FALSE(test(' '));
}

TEST(RequestDecoderInternalsTest, FindFirstNotOf) {
  EXPECT_EQ(FindFirstNotOf(StringView(" HTTP/1.1"), IsQueryChar), 0);
}

}  // namespace
}  // namespace test
}  // namespace mcucore_http1_internal
}  // namespace http1
}  // namespace mcucore
