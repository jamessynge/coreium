#include "crc32.h"

// Given that a CRC algorithm appears to output a psuedo-random value given some
// input, how should it be tested? Some possibilities:
//
// * Test with inputs of various special numbers of bytes (0, 1, lots)
// * Compare two implementations of the algorithm that use the same polynomial,
//   reversing behavior, endianness, etc. A variant of that is to compare this
//   implementation against values previously calculated by this or another
//   implementation to confirm that the value calculated is unchanged.
// * Test the ability to detect N-bit errors by generating random data,
//   calculate the crc, then swap N randomly selected bits in the random data,
//   and confirm that the implementation generates a different crc value after
//   that corruption.
//
// NOTE: I'm rather skeptical that "my" CRC-32 implementation (cribbed from the
// Arduino site, which in turn has copied from elsewhere) is "correct". It does
// appear to be producing CRC values, but they don't match the values produced
// by online CRC calculators.

#include <algorithm>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "extras/test_tools/random_utils.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

auto CalculateCrc(const std::vector<uint8_t>& input) {
  Crc32 crc;
  for (const auto v : input) {
    crc.appendByte(v);
  }
  return crc.value();
}

auto CalculateCrc(std::string_view str) {
  Crc32 crc;
  for (const auto c : str) {
    crc.appendByte(static_cast<uint8_t>(c));
  }
  return crc.value();
}

TEST(Crc32Test, Empty) {
  Crc32 crc;
  EXPECT_EQ(crc.value(), static_cast<uint32_t>(~0L));
}

TEST(Crc32Test, FixedStrings) {
  const uint32_t kAddrCrc = 1640883860;
  {
    Crc32 crc;
    crc.appendByte('A');
    crc.appendByte('d');
    crc.appendByte('d');
    crc.appendByte('r');
    EXPECT_EQ(crc.value(), kAddrCrc);
  }

  // Addr again, but using a helper.
  EXPECT_EQ(CalculateCrc("Addr"), kAddrCrc);
  EXPECT_EQ(CalculateCrc("10.1.2.3"), 2931413026);

  // Addr again, but using a different helper.
  EXPECT_EQ(CalculateCrc({0x41, 0x64, 0x64, 0x72}), kAddrCrc);
}

////////////////////////////////////////////////////////////////////////////////
// General approach to checking that corruption is detected:
// * Generate a random size.
// * Generate |size| random bytes.
// * Calculate the crc of those bytes.
// * Corrupt N bits of those bytes.
// * Calculate the crc of the corrupted bytes.
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

class Crc32RandomTest : public testing::TestWithParam<DetectsCorruptionParams> {
 public:
  Crc32RandomTest() : prng_(GetTestCaseSeed()) {}

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

    const auto original_crc = CalculateCrc(original_bytes);
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
      const auto new_crc = CalculateCrc(corrupted);
      EXPECT_NE(original_crc, new_crc)
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
  const auto kNumTrials = 8;
  for (size_t num_data_bytes = 1; num_data_bytes <= 32; ++num_data_bytes) {
    // This is too many trials when Crc32 logs a lot.
    //    const auto kNumTrials = static_cast<int>(num_data_bytes * 8);
    for (int num_bits_to_flip = 1; num_bits_to_flip <= 4; ++num_bits_to_flip) {
      cases.push_back({num_data_bytes, num_bits_to_flip, kNumTrials});
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

INSTANTIATE_TEST_SUITE_P(DetectsCorruption, Crc32RandomTest,
                         testing::ValuesIn(GenerateTestParams()),
                         GenerateTestName);

TEST_P(Crc32RandomTest, DetectsCorruption) {
  const auto original_bytes = GenerateBytes(GetParam().num_data_bytes);
  DetectCorruption(original_bytes, GetParam().num_bits_to_flip,
                   GetParam().num_trials);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
