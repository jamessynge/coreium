#ifndef MCUCORE_EXTRAS_TEST_TOOLS_RANDOM_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_RANDOM_UTILS_H_

// Encapsulates configuring random number generators. This is useful because we
// want reproducibility of test results, i.e. if a test using a truly random
// seed fails, we then want to be able to re-run the test with the same seed to
// better understand the cause and to validate our fix.
//
// Given that this is for testing Arduino related code, a uint32_t is plenty big
// enough as the size of our seed and the generated random values.
//
// Author: james.synge@gmail.com

#include <cstdint>

namespace mcucore {
namespace test {

// If a specific flag is set, returns the value of that flag every time; else
// LOGS and returns a randomly generated value each time. Intended to be called
// at most once per test case, for use as the seed for a PRNG used by the test.
uint32_t GetTestCaseSeed();

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_RANDOM_UTILS_H_
