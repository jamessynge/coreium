#include "hash/fnv1a.h"

// Given that a hash algorithm appears to output a psuedo-random value given
// some input, how should it be tested? Some possibilities:
//
// * Test with inputs of various special numbers of bytes (0, 1, lots)
// * Compare two implementations of the same algorithm, with the same special
//   values (e.g. the FNV-1a Prime), endianness, etc. A variant of that is to
//   compare this implementation against values previously calculated by this or
//   another implementation to confirm that the value calculated is unchanged.
// * Test the ability to detect N-bit errors by generating random data,
//   calculate the hash, then swap N randomly selected bits in the random data,
//   and confirm that the implementation generates a different hash value after
//   that corruption (see the Avalanche Effect on wikipedia, w.r.t. crypto).

#include <algorithm>
#include <array>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/random_utils.h"
#include "extras/test_tools/sample_printable.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log.h"

namespace mcucore {
namespace test {
namespace {

template <typename T = std::vector<uint8_t>>
uint32_t CalculateHash(const T& input) {
  Fnv1a hasher;
  for (const auto elem : input) {
    hasher.appendByte(static_cast<uint8_t>(elem));
  }
  return hasher.value();
}

uint32_t CalculateHash(const char* str) {
  return CalculateHash(std::string_view(str));
}

// auto CalculateHash(const std::vector<uint8_t>& input) {
//   Fnv1a hasher;
//   for (const auto v : input) {
//     hasher.appendByte(v);
//   }
//   return hasher.value();
// }

// auto CalculateHash(std::string_view str) {
//   Fnv1a hasher;
//   for (const auto c : str) {
//     hasher.appendByte(static_cast<uint8_t>(c));
//   }
//   return hasher.value();
// }

TEST(Fnv1aTest, Empty) {
  Fnv1a hasher;
  EXPECT_EQ(hasher.value(), 2166136261);  // FNV 32 offset basis.
}

TEST(Fnv1aTest, FixedStrings) {
  const uint32_t kExpectedHash = 0xc1053e92;
  {
    Fnv1a hasher;
    hasher.appendByte('A');
    hasher.appendByte('d');
    hasher.appendByte('d');
    hasher.appendByte('r');
    EXPECT_EQ(hasher.value(), kExpectedHash);
  }

  // Addr again, but using a helper.
  EXPECT_EQ(CalculateHash("Addr"), kExpectedHash);
  EXPECT_EQ(CalculateHash({0x41, 0x64, 0x64, 0x72}), kExpectedHash);

  // Another fixed string.
  EXPECT_EQ(CalculateHash("10.1.2.3"), 0x3987d90a);
}

TEST(Fnv1aTest, LowCollisionCount) {
  // We expect to use this to generate the seed for a PRNG based on a sequence
  // of bytes produced by reading Timer/Counter 0 (8-bits) everytime the
  // Watchdog Timer (WDT) interrupt fires, for some number of interrupts (e.g. 6
  // to 20). T/C 0 wraps around every 1.024 milliseconds, and the WDT interrupts
  // (very) approximately every 16ms. What can we expect regarding the diversity
  // of T/C0 values produced? While we expect that the WDT is temperature
  // dependent, and varies over longer time scales, we don't know that it isn't
  // somewhat stable over shorter periods (e.g. 1 second).

  // Until I have better data, I suppose that I should assume that we'll
  // generate N random bytes. So, if we generate M such sequences, do any of
  // them produce the same hash?

  std::mt19937 prng(GetTestCaseSeed());
  std::uniform_int_distribution<size_t> distribution(0, 255);

  constexpr size_t kNumBytesPerSample = 20;
  const size_t kNumSequences = 1000000;
  const size_t kMinWithoutCollisions = 5000;  // At most one collision per...
  using Value = std::array<uint8_t, kNumBytesPerSample>;

  size_t num_collisions = 0;
  absl::btree_map<uint32_t, std::vector<Value>> collisions;
  {
    absl::flat_hash_map<uint32_t, Value> hash_to_value;
    hash_to_value.reserve(kNumSequences);
    for (size_t i = 0; i < kNumSequences; ++i) {
      Value value;
      for (size_t j = 0; j < kNumBytesPerSample; ++j) {
        value[j] = distribution(prng);
      }
      const auto hash = CalculateHash(value);
      if (!hash_to_value.contains(hash)) {
        hash_to_value[hash] = value;
        continue;
      }
      // We have the same hash value already. Did we accidently generate the
      // same input value as before?
      if (hash_to_value[hash] == value) {
        // Yup, so not a collision.
        continue;
      }
      LOG(WARNING) << "Two random values hashed to the same value (" << hash
                   << "):\n First: " << absl::StrJoin(hash_to_value[hash], ", ")
                   << "\nSecond: " << absl::StrJoin(value, ", ");
      ++num_collisions;
      if (!collisions.contains(hash)) {
        collisions[hash].push_back(hash_to_value[hash]);
      }
      collisions[hash].push_back(value);
    }
  }
  LOG(INFO) << num_collisions << " collisions in " << kNumSequences
            << " randomly generated values.";
  for (const auto& [hash, values] : collisions) {
    LOG(INFO) << "Collisions for hash value " << hash << " (" << values.size()
              << "):\n"
              << absl::StrJoin(values, "\n",
                               [](std::string* out, const Value& value) {
                                 absl::StrAppend(out,
                                                 absl::StrJoin(value, ", "));
                               })
              << "\n";
  }

  LOG(INFO) << num_collisions << " collisions in " << kNumSequences
            << " randomly generated values.";
  EXPECT_GE((static_cast<double>(kNumSequences) / num_collisions),
            kMinWithoutCollisions);
}

////////////////////////////////////////////////////////////////////////////////
// General approach to checking that corruption is detected:
// * Generate a random size.
// * Generate |size| random bytes.
// * Calculate the hash of those bytes.
// * Corrupt N bits of those bytes.
// * Calculate the hash of the corrupted bytes.
// * Confirm they're different.
//
// We use a parameterized test approach to conducting these tests.

size_t NChooseK(const size_t N, const size_t K) {
  size_t result;
  if (K == 0) {
    result = 1;
  } else if (K > N) {
    result = 0;
  } else {
    const size_t limit = std::min(K, N - K);
    size_t numerator = N;
    size_t denominator = 1;
    for (size_t i = 2; i <= limit; ++i) {
      numerator *= (N + 1 - i);
      denominator *= i;
    }
    result = numerator / denominator;
  }
  return result;
}

TEST(UtilTest, NChooseK) {
  EXPECT_EQ(NChooseK(1, 0), 1);
  EXPECT_EQ(NChooseK(1, 1), 1);
  EXPECT_EQ(NChooseK(1, 2), 0);

  EXPECT_EQ(NChooseK(8, 1), (8) / (1));
  EXPECT_EQ(NChooseK(8, 2), (8 * 7) / (2 * 1));
  EXPECT_EQ(NChooseK(8, 3), (8 * 7 * 6) / (3 * 2 * 1));

  EXPECT_EQ(NChooseK(16, 1), (16) / (1));
  EXPECT_EQ(NChooseK(16, 2), (16 * 15) / (2 * 1));
}

void FlipBit(size_t bit_number, std::vector<uint8_t>& bytes) {
  size_t byte_number = bit_number / 8;
  uint8_t bit_mask = 1 << (bit_number % 8);
  bytes[byte_number] ^= bit_mask;
}

struct DetectsCorruptionParams {
  // If we wanted to be exhaustive (i.e. all examples of N choose K), we could
  // indicate that here with either a new field, or with num_trials == 0.
  size_t num_data_bytes;
  int num_bits_to_flip;
  int num_trials;
};

class Fnv1aRandomTest : public testing::TestWithParam<DetectsCorruptionParams> {
 public:
  Fnv1aRandomTest() : prng_(GetTestCaseSeed()) {}

 protected:
  std::vector<uint8_t> GenerateBytes(const size_t size) {
    std::vector<uint8_t> result;
    result.reserve(size);
    while (result.size() < size) {
      result.push_back(static_cast<uint32_t>(prng_() % 0xff));
    }
    return result;
  }

  std::set<size_t> GenerateUniqueInts(const int size, const size_t limit) {
    EXPECT_GT(size, 0);
    EXPECT_GT(limit, 0);
    EXPECT_GT(limit, size);
    std::uniform_int_distribution<size_t> distribution(0, limit - 1);
    std::set<size_t> result;
    while (result.size() < size) {
      result.insert(distribution(prng_));
    }
    return result;
  }

  // Given one sample of data, corrupt it |trials| times, checking if the CRC
  // detects the corruption each time.
  void DetectCorruption(const std::vector<uint8_t>& original_bytes,
                        const int num_bits_to_flip, const int num_trials) {
    // Make sure we're not trying to do more unique trials than are possible.
    const auto max_possible =
        NChooseK(original_bytes.size() * 8, num_bits_to_flip);
    ASSERT_LE(num_trials, max_possible);

    const auto original_hash = CalculateHash(original_bytes);
    std::set<std::set<size_t>> tested_corruptions;
    std::vector<uint8_t> corrupted;

    for (int trial = 0; trial < num_trials; ++trial) {
      auto bits_to_corrupt =
          GenerateUniqueInts(num_bits_to_flip, original_bytes.size() * 8);
      if (!tested_corruptions.insert(bits_to_corrupt).second) {
        // Already tested this case.
        --trial;
        continue;
      }
      corrupted = original_bytes;
      for (const auto bit_number : bits_to_corrupt) {
        FlipBit(bit_number, corrupted);
      }
      const auto new_hash = CalculateHash(corrupted);
      EXPECT_NE(original_hash, new_hash)
          << "\n"
          << "original_bytes: [" << absl::StrJoin(original_bytes, ", ") << "]\n"
          << "num_bits_to_flip: " << num_bits_to_flip << "\n"
          << "trial: " << trial << "\n"
          << "bits_to_corrupt: {" << absl::StrJoin(bits_to_corrupt, ", ")
          << "}\n"
          << "corrupted: [" << absl::StrJoin(corrupted, ", ") << "]";
    }
  }

  std::mt19937 prng_;
};

std::vector<DetectsCorruptionParams> GenerateTestParams() {
  std::vector<DetectsCorruptionParams> cases;
  [[maybe_unused]] const auto kMinimumNumTrials = 8;
  for (size_t num_data_bytes = 1; num_data_bytes <= 32; ++num_data_bytes) {
    // This is too many trials at larger numbers of bytes if Fnv1a logs a lot.
    int num_trials = static_cast<int>(num_data_bytes * 8);
#if defined(MCU_ENABLE_DCHECK) || \
    (defined(MCU_ENABLED_VLOG_LEVEL) && MCU_ENABLED_VLOG_LEVEL > 3)
    num_trials = kMinimumNumTrials;
#endif
    for (int num_bits_to_flip = 1; num_bits_to_flip <= 4; ++num_bits_to_flip) {
      cases.push_back({num_data_bytes, num_bits_to_flip, num_trials});
    }
  }
  return cases;
}

std::string GenerateTestName(
    const testing::TestParamInfo<DetectsCorruptionParams>& info) {
  const auto& param = info.param;
  return absl::StrCat(param.num_data_bytes, "B_", param.num_bits_to_flip, "b_",
                      param.num_trials);
}

INSTANTIATE_TEST_SUITE_P(DetectsCorruption, Fnv1aRandomTest,
                         testing::ValuesIn(GenerateTestParams()),
                         GenerateTestName);

TEST_P(Fnv1aRandomTest, DetectsCorruption) {
  const auto original_bytes = GenerateBytes(GetParam().num_data_bytes);
  DetectCorruption(original_bytes, GetParam().num_bits_to_flip,
                   GetParam().num_trials);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
