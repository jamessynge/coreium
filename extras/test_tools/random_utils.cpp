#include "extras/test_tools/random_utils.h"

#include <random>

#include "absl/flags/flag.h"
#include "absl/log/log.h"

ABSL_FLAG(uint32_t, mcucore_random_seed, 0, "Random seed.");

namespace mcucore {
namespace test {

uint32_t GetTestCaseSeed() {
  static uint32_t seed = absl::GetFlag(FLAGS_mcucore_random_seed);

  if (seed != 0) {
    // The seed has been specified via command-line, and never changes during
    // the run. This means that it is only useful for testing a re-running a
    // single test case, as normally each test case which calls GetTestCaseSeed
    // will get a unique response, so they can't all be replayed at the same
    // time with a single seed.
  } else {
    // We need to generate a new random seed (i.e. a random value). We'll use
    // random_device to seed a PRNG once only, then use that PRNG to generate
    // one random value on each call.
    static auto* maybe_hardware_random = new std::random_device();
    static auto* generator = new std::mt19937((*maybe_hardware_random)());
    seed = (*generator)();
  }
  LOG(INFO) << "GetTestCaseSeed -> " << seed;
  return seed;
}

}  // namespace test
}  // namespace mcucore
