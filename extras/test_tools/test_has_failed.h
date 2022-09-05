#ifndef MCUCORE_EXTRAS_TEST_TOOLS_TEST_HAS_FAILED_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_TEST_HAS_FAILED_H_

// A helper function for test cases that may have lots of expectations, where if
// any fail, then many are likely to fail (e.g. testing in a loop with many
// different values). To avoid spewing lots of output, TestHasFailed() allows a
// test to decide to exit early.

#include "gtest/gtest.h"

namespace mcucore {
namespace test {

inline bool TestHasFailed() {
  auto test_info = testing::UnitTest::GetInstance()->current_test_info();
  return test_info->result()->Failed();
}

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_TEST_HAS_FAILED_H_
