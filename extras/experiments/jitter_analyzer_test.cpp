#include "extras/experiments/jitter_analyzer.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <map>
#include <ostream>
#include <string>
#include <system_error>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAre;

TEST(SingleRampTimerCounterTest, VerifyRamp) {
  SingleRampTimerCounter tc(0, 0, 1, 5);

  EXPECT_EQ(tc.ReadCounter(0), 0);
  EXPECT_EQ(tc.ReadCounter(1), 1);
  EXPECT_EQ(tc.ReadCounter(2), 2);
  EXPECT_EQ(tc.ReadCounter(3), 3);
  EXPECT_EQ(tc.ReadCounter(4), 4);
  EXPECT_EQ(tc.ReadCounter(5), 5);
  EXPECT_EQ(tc.ReadCounter(6), 0);
  EXPECT_EQ(tc.ReadCounter(7), 1);
  EXPECT_EQ(tc.ReadCounter(8), 2);
  EXPECT_EQ(tc.ReadCounter(9), 3);
  EXPECT_EQ(tc.ReadCounter(10), 4);
  EXPECT_EQ(tc.ReadCounter(11), 5);
  EXPECT_EQ(tc.ReadCounter(12), 0);
  EXPECT_EQ(tc.ReadCounter(13), 1);
  EXPECT_EQ(tc.ReadCounter(14), 2);
}

TEST(SingleRampTimerCounterTest, VerifyRampScaledAndOffset) {
  constexpr uint64_t kStartingCpuTime = 5;
  constexpr uint16_t kStartingClockTicks = 2;
  constexpr uint32_t kCpuTicksPerClockTick = 2;
  constexpr uint16_t kMaxClockTicks = 3;

  SingleRampTimerCounter tc(kStartingCpuTime, kStartingClockTicks,
                            kCpuTicksPerClockTick, kMaxClockTicks);

  uint16_t expected_counter = kStartingClockTicks;
  for (uint64_t cpu_time_at_start_of_clock_tick = kStartingCpuTime;
       cpu_time_at_start_of_clock_tick < 50;
       cpu_time_at_start_of_clock_tick += kCpuTicksPerClockTick) {
    for (uint32_t cpu_time_within_clock_tick = 0;
         cpu_time_within_clock_tick < kCpuTicksPerClockTick;
         ++cpu_time_within_clock_tick) {
      const auto cpu_time =
          cpu_time_at_start_of_clock_tick + cpu_time_within_clock_tick;
      EXPECT_EQ(tc.ReadCounter(cpu_time), expected_counter)
          << "\ncpu_time = " << cpu_time;
    }

    if (expected_counter == kMaxClockTicks) {
      expected_counter = 0;
    } else {
      ++expected_counter;
    }
  }
}

TEST(DualRampTimerCounterTest, VerifyRamp) {
  DualRampTimerCounter tc(0, 0, 1, 3);

  EXPECT_EQ(tc.ReadCounter(0), 0);
  EXPECT_EQ(tc.ReadCounter(1), 1);
  EXPECT_EQ(tc.ReadCounter(2), 2);
  EXPECT_EQ(tc.ReadCounter(3), 3);
  EXPECT_EQ(tc.ReadCounter(4), 2);
  EXPECT_EQ(tc.ReadCounter(5), 1);
  EXPECT_EQ(tc.ReadCounter(6), 0);
  EXPECT_EQ(tc.ReadCounter(7), 1);
  EXPECT_EQ(tc.ReadCounter(8), 2);
  EXPECT_EQ(tc.ReadCounter(9), 3);
  EXPECT_EQ(tc.ReadCounter(10), 2);
  EXPECT_EQ(tc.ReadCounter(11), 1);
  EXPECT_EQ(tc.ReadCounter(12), 0);
  EXPECT_EQ(tc.ReadCounter(13), 1);
  EXPECT_EQ(tc.ReadCounter(14), 2);
}

TEST(DualRampTimerCounterTest, VerifyRampScaledAndOffset) {
  constexpr uint64_t kStartingCpuTime = 7;
  constexpr uint16_t kStartingClockTicks = 3;
  constexpr uint32_t kCpuTicksPerClockTick = 4;
  constexpr uint16_t kMaxClockTicks = 4;

  DualRampTimerCounter tc(kStartingCpuTime, kStartingClockTicks,
                          kCpuTicksPerClockTick, kMaxClockTicks);

  bool rising = true;
  uint16_t expected_counter = kStartingClockTicks;
  for (uint64_t cpu_time_at_start_of_clock_tick = kStartingCpuTime;
       cpu_time_at_start_of_clock_tick < 100;
       cpu_time_at_start_of_clock_tick += kCpuTicksPerClockTick) {
    for (uint32_t cpu_time_within_clock_tick = 0;
         cpu_time_within_clock_tick < kCpuTicksPerClockTick;
         ++cpu_time_within_clock_tick) {
      const auto cpu_time =
          cpu_time_at_start_of_clock_tick + cpu_time_within_clock_tick;
      EXPECT_EQ(tc.ReadCounter(cpu_time), expected_counter)
          << "\ncpu_time = " << cpu_time;
    }

    if (expected_counter == 0 || expected_counter == kMaxClockTicks) {
      rising = !rising;
    }

    if (rising) {
      ++expected_counter;
    } else {
      --expected_counter;
    }
  }
}

TEST(MillisToCpuTimeTest, Basic) {
  EXPECT_EQ(MillisToCpuTime(1000.0), 16000000);
  EXPECT_EQ(MillisToCpuTime(1), 16000);
  EXPECT_EQ(MillisToCpuTime(16), 256000);
}

std::string ToChars(double ms) {
  // Based on: https://en.cppreference.com/w/cpp/utility/to_chars
  std::array<char, 30> str;
  if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), ms,
                                     std::chars_format::fixed);
      ec == std::errc()) {
    return std::string(str.data(), ptr);
  } else {
    return std::make_error_code(ec).message();
  }
}

TEST(PickCpuTimeSamplesNearTest, DumpForStudy) {
  const auto kMean = MillisToCpuTime(16);
  const auto kStdDev = MillisToCpuTime(0.8);
  const auto kSamples = 10000;

  const auto describe_range = [&](uint64_t below, uint64_t above) {
    for (int relative = -5; relative <= 5; ++relative) {
      const auto time = kMean + relative * kStdDev;
      if (below <= time && time < above) {
        if (relative == 0) {
          std::cout << "MEAN";
        } else {
          std::cout << relative << " * stddev";
        }
        std::cout << " (" << time << " cycles, "
                  << ToChars(CpuTimeToMillis(time)) << "ms)" << std::endl;
      }
    }
  };

  auto values = PickCpuTimeSamplesNear(kMean, kStdDev, kSamples);
  std::sort(values.begin(), values.end());
  uint64_t last = 0;

  for (const auto value : values) {
    describe_range(last, value);

    if (value < (kMean - 4 * kStdDev) || value > (kMean + 4 * kStdDev)) {
      std::cout << value;
      if (last != 0) {
        EXPECT_GE(value, last);
        if (last == value) {
          std::cout << "    DUPLICATE";
        } else {
          std::cout << "    +" << (value - last);
        }
      }
      std::cout << std::endl;
    }
    last = value;
  }
  describe_range(last, last * 2);
}

TEST(ReadCountersTest, OneCounter) {
  const uint16_t kMaxClockTicks = 65535;
  SingleRampTimerCounter counter(0, 0, 1, kMaxClockTicks);
  const std::vector<const CounterInterface*> counters = {&counter};
  for (uint64_t cpu_time = 0; cpu_time < 100000; cpu_time += 99) {
    EXPECT_THAT(ReadCounters(cpu_time, counters),
                ElementsAre(cpu_time % (kMaxClockTicks + 1L)));
  }
}

TEST(ReadCountersTest, TwoCounters) {
  const uint16_t kMaxClock1Ticks = 255;
  const uint16_t kMaxClock2Ticks = 65535;
  SingleRampTimerCounter counter1(0, 0, 1, kMaxClock1Ticks);
  SingleRampTimerCounter counter2(0, 0, 1, kMaxClock2Ticks);
  const std::vector<const CounterInterface*> counters = {&counter1, &counter2};
  for (uint64_t cpu_time = 0; cpu_time < 100000; cpu_time += 98) {
    EXPECT_THAT(ReadCounters(cpu_time, counters),
                ElementsAre(cpu_time % (kMaxClock1Ticks + 1L),
                            cpu_time % (kMaxClock2Ticks + 1L)));
  }
}

TEST(ReadCountersAtTimesTest, OneCounterTwice) {
  const uint16_t kMaxClockTicks = 65535;
  SingleRampTimerCounter counter(0, 0, 1, kMaxClockTicks);
  const std::vector<const CounterInterface*> counters = {&counter};
  for (uint64_t cpu_time = 0; cpu_time < 100000; cpu_time += 99) {
    EXPECT_THAT(
        ReadCountersAtTimes(cpu_time, cpu_time + 1, 1, counters),
        ElementsAre(ElementsAre(cpu_time % (kMaxClockTicks + 1L)),
                    ElementsAre((cpu_time + 1) % (kMaxClockTicks + 1L))));
  }
}

TEST(ReadCountersAtTimesTest, TwoCountersTwice) {
  const uint64_t kCpuTimeSpacing = 11;
  const uint64_t kLastTimeOffset = kCpuTimeSpacing - 1 + kCpuTimeSpacing / 2;
  const uint16_t kMaxClock1Ticks = 255;
  const uint16_t kMaxClock2Ticks = 65535;
  SingleRampTimerCounter counter1(0, 0, 1, kMaxClock1Ticks);
  SingleRampTimerCounter counter2(0, 0, 1, kMaxClock2Ticks);
  const std::vector<const CounterInterface*> counters = {&counter1, &counter2};
  for (uint64_t cpu_time = 0; cpu_time < 100000; cpu_time += 98) {
    auto at_time0 = ElementsAre(cpu_time % (kMaxClock1Ticks + 1L),
                                cpu_time % (kMaxClock2Ticks + 1L));
    auto time1 = cpu_time + kCpuTimeSpacing;
    auto at_time1 = ElementsAre(time1 % (kMaxClock1Ticks + 1L),
                                time1 % (kMaxClock2Ticks + 1L));
    EXPECT_THAT(ReadCountersAtTimes(cpu_time, cpu_time + kLastTimeOffset,
                                    kCpuTimeSpacing, counters),
                ElementsAre(at_time0, at_time1));
  }
}

// TODO(jamessynge): Add tests of VisitCounterValuesAtTimes.

TEST(EntropyOfDiscreteDistributionTest, VerifyUniform) {
  EXPECT_NEAR(EntropyOfDiscreteDistribution({0, 1}), 1.0, 0.00001);
  EXPECT_NEAR(EntropyOfDiscreteDistribution({0, 10, 11, 20}), 2.0, 0.00001);
  EXPECT_NEAR(EntropyOfDiscreteDistribution({0, 0, 0, 0, 0, 0, 0, 1, 1, 1}),
              0.881, 0.001);
}

TEST(EntropyOfDiscreteDistributionTest, AvrExpected) {
  // I'm assuming here that kPercentPlusMinus represents 3 standard deviations,
  // so 99.7% of the samples will appear with in plus or minus kPercentPlusMinus
  // of the central value.

  const auto kCentralCpuTime = MillisToCpuTime(16);
  const auto kPercentPlusMinus = 10;
  const auto kCpuTimePlusMinus = kCentralCpuTime * kPercentPlusMinus / 100;
  const auto kStandardDeviation = kCpuTimePlusMinus / 3;
  const auto kNumSamples = 1000 * 1000;

  std::cout << kCentralCpuTime << " +/- " << kCpuTimePlusMinus << " (stddev "
            << kStandardDeviation << ")" << std::endl;

  auto all_samples =
      PickCpuTimeSamplesNear(kCentralCpuTime, kStandardDeviation, kNumSamples);

  std::vector<uint64_t> trimmed_samples;
  std::copy_if(all_samples.begin(), all_samples.end(),
               std::back_inserter(trimmed_samples), [&](uint64_t sample) {
                 return (kCentralCpuTime - kCpuTimePlusMinus) <= sample &&
                        sample <= (kCentralCpuTime + kCpuTimePlusMinus);
               });
  std::cout << "trimmed_samples size: " << trimmed_samples.size() << std::endl;

  std::cout << "Entropy: " << EntropyOfDiscreteDistribution(trimmed_samples)
            << std::endl
            << std::flush;
}

// Measure how unique the T/C counter samples are. We record how many times each
// sample is generated (the "occurrence count" for a sample), then we record how
// many times each "occurrence count" itself occurs; we desire that each
// occurrence count is seen just once, which would indicate that we are
// generating entirely unique samples, which suits us well for seeding the RNG.
void MeasureUniquenessOfSamples(
    uint64_t first_cpu_time, uint64_t last_cpu_time, uint64_t cpu_time_spacing,
    const std::vector<const CounterInterface*>& counters) {
  absl::flat_hash_map<std::vector<uint16_t>, int> sample_occurrences;
  sample_occurrences.reserve(
      (last_cpu_time - first_cpu_time) / cpu_time_spacing + 1);
  size_t num_samples = 0;
  auto visitor = [&](const std::vector<uint16_t>& sample) {
    sample_occurrences[sample]++;
    num_samples++;
  };
  VisitCounterValuesAtTimes(first_cpu_time, last_cpu_time, 1, counters,
                            visitor);

  std::map<int, int> occurrence_counts;
  for (const auto& entry : sample_occurrences) {
    occurrence_counts[entry.second]++;
  }

  if (occurrence_counts.size() == 1) {
    std::cout << "Across " << num_samples << ", all were unique" << std::endl;
  } else {
    for (const auto& entry : occurrence_counts) {
      std::cout << entry.first << " " << entry.second << std::endl;
    }
  }
}

// Not really variance, just the % difference from the central time of the low
// and high time values.
void MeasureUniquenessOfSamplesWithPercentVariance(
    uint64_t central_cpu_time, int percent_variance,
    const std::vector<const CounterInterface*>& counters) {
  const auto offset = central_cpu_time * percent_variance / 100;
  const auto first_cpu_time = central_cpu_time - offset;
  const auto last_cpu_time = central_cpu_time + offset;
  MeasureUniquenessOfSamples(first_cpu_time, last_cpu_time, 1, counters);
}

// NOT a test, just prints values for study.
TEST(GenerateSamplesTest, Case10PercentVarianceWith12BitTimers) {
  const auto kCentralCpuTime = MillisToCpuTime(16);
  const auto kPercentVariance = 10;

  // Assuming here that T/C #0 is reserved for Arduino; the others are set to
  // have TOP as a unique prime number below MAX. For the 16-bit T/Cs, we choose
  // to treat them as almost 12 bit counters, and use the first 4 primes below
  // 2048 as their' TOPs.
  SingleRampTimerCounter counter0(0, 0, 1, 255);
  SingleRampTimerCounter counter1(0, 0, 1, 2039);  // First prime below 2048
  SingleRampTimerCounter counter2(0, 0, 1, 251);   // First prime below 256.
  SingleRampTimerCounter counter3(0, 0, 1, 2029);  // Second prime below 2048
  SingleRampTimerCounter counter4(0, 0, 1, 2027);  // Third prime below 2048
  SingleRampTimerCounter counter5(0, 0, 1, 2017);  // Fourth prime below 2048

  const std::vector<const CounterInterface*> counters = {
      &counter0, &counter1, &counter2, &counter3, &counter4, &counter5};
  EXPECT_THAT(counters, testing::SizeIs(6));

  MeasureUniquenessOfSamplesWithPercentVariance(kCentralCpuTime,
                                                kPercentVariance, counters);
}

// NOT a test, just prints values for study.
TEST(GenerateSamplesTest, Case30PercentVarianceWith8BitTimers) {
  const auto kCentralCpuTime = MillisToCpuTime(16);
  const auto kPercentVariance = 30;

  // If we restrict ourselves to reading only 8-bit values (i.e. only the low
  // 8-bits of the 16 bit counters), do we still get unique values?
  SingleRampTimerCounter counter0(0, 0, 1, 255);
  SingleRampTimerCounter counter1(0, 0, 1, 251);  // First prime below 256.
  SingleRampTimerCounter counter2(0, 0, 1, 241);  // Second prime below 256.
  SingleRampTimerCounter counter3(0, 0, 1, 239);  // Third prime below 256.
  SingleRampTimerCounter counter4(0, 0, 1, 233);  // Fourth prime below 256.
  SingleRampTimerCounter counter5(0, 0, 1, 229);  // Fifth prime below 256.

  const std::vector<const CounterInterface*> counters = {
      &counter0, &counter1, &counter2, &counter3, &counter4, &counter5};
  EXPECT_THAT(counters, testing::SizeIs(6));

  MeasureUniquenessOfSamplesWithPercentVariance(kCentralCpuTime,
                                                kPercentVariance, counters);
}

// NOT a test, just prints values for study.
TEST(GenerateSamplesTest, Case5PercentVarianceWithVaryingCounters) {
  const auto kCentralCpuTime = MillisToCpuTime(16);
  const auto kPercentVariance = 5;

  // If we restrict ourselves to reading only 8-bit values (i.e. only the
  // low 8-bits of the 16 bit counters), do we still get unique values?
  SingleRampTimerCounter counter0(0, 0, 1, 255);
  SingleRampTimerCounter counter1(0, 0, 1, 251);  // 1st prime below 256.
  SingleRampTimerCounter counter2(0, 0, 1, 241);  // 2nd prime below 256.
  SingleRampTimerCounter counter3(0, 0, 1, 239);  // 3rd prime below 256.
  SingleRampTimerCounter counter4(0, 0, 1, 233);  // 4th prime below 256.
  SingleRampTimerCounter counter5(0, 0, 1, 229);  // 5th prime below 256.

  const std::vector<const CounterInterface*> all_counters = {
      &counter0, &counter1, &counter2, &counter3, &counter4, &counter5};
  EXPECT_THAT(all_counters, testing::SizeIs(6));

  for (int num_counters = 1; num_counters <= all_counters.size();
       ++num_counters) {
    std::cout << std::endl << "num_counters: " << num_counters << std::endl;
    std::vector<const CounterInterface*> counters = all_counters;
    // Keep only the first `num_counters` Counters.
    counters.resize(num_counters);
    MeasureUniquenessOfSamplesWithPercentVariance(kCentralCpuTime,
                                                  kPercentVariance, counters);
  }
}

}  // namespace
