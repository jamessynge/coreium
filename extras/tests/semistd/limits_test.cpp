#include "semistd/limits.h"

#include <limits>

#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

template <typename T>
void VerifyLimits() {
  EXPECT_TRUE((is_same<T, decltype(numeric_limits<T>::min())>::value));
  EXPECT_TRUE((is_same<T, decltype(numeric_limits<T>::max())>::value));

  EXPECT_EQ(numeric_limits<T>::min(), std::numeric_limits<T>::min());
  EXPECT_EQ(numeric_limits<T>::max(), std::numeric_limits<T>::max());

  EXPECT_EQ(numeric_limits<T>::is_signed, std::numeric_limits<T>::is_signed);
  EXPECT_EQ(numeric_limits<T>::is_exact, std::numeric_limits<T>::is_exact);
}

TEST(LimitsTest, VerifyLimits) {
  VerifyLimits<char>();
  VerifyLimits<signed char>();
  VerifyLimits<unsigned char>();

  VerifyLimits<signed short>();    // NOLINT
  VerifyLimits<unsigned short>();  // NOLINT

  VerifyLimits<signed int>();    // NOLINT
  VerifyLimits<unsigned int>();  // NOLINT

  VerifyLimits<signed long>();    // NOLINT
  VerifyLimits<unsigned long>();  // NOLINT

  VerifyLimits<int8_t>();
  VerifyLimits<uint8_t>();

  VerifyLimits<int16_t>();
  VerifyLimits<uint16_t>();

  VerifyLimits<int32_t>();
  VerifyLimits<uint32_t>();
}

}  // namespace
}  // namespace test
}  // namespace mcucore
