#include "extras/experiments/jitter_analyzer.h"

// If we use the CPU clock and the Watchdog clock to generate random numbers
// (or the seed for an RNG) by measuring the jitter between the two, how random
// are they likely to be? More specifically, if we repeat this process many
// times, how many unique values are we likely to read from the Timer/Counter
// peripherals? If reading an 8-bit T/C, is each of the 256 possible values
// equally likely? If reading a 16-bit T/C, are the values returned equally
// likely?
//
// NOTE: It isn't clear to me that the T/C counter register has each value for
// the same amount of time. In particular, it isn't clear whether the BOTTOM (0)
// or TOP (settable) value is held for a full clock tick in all modes.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>

#include "absl/log/check.h"

// CPU cycles per second, nominal.
constexpr uint32_t kCpuCyclesPerSecond = 16000000;

uint64_t MillisToCpuTime(double ms, uint32_t cpu_cycles_per_second) {
  if (cpu_cycles_per_second == 0) {
    cpu_cycles_per_second = kCpuCyclesPerSecond;
  }
  return static_cast<uint64_t>(std::round((ms * kCpuCyclesPerSecond) / 1000.0));
}

double CpuTimeToMillis(uint64_t cpu_time, uint32_t cpu_cycles_per_second) {
  if (cpu_cycles_per_second == 0) {
    cpu_cycles_per_second = kCpuCyclesPerSecond;
  }
  return cpu_time * 1000.0 / static_cast<double>(cpu_cycles_per_second);
}

std::vector<uint16_t> ReadCounters(
    uint64_t cpu_time, const std::vector<const CounterInterface*>& counters) {
  std::vector<uint16_t> results;
  results.reserve(counters.size());
  for (const CounterInterface* counter : counters) {
    results.push_back(counter->ReadCounter(cpu_time));
  }
  return results;
}

std::vector<std::vector<uint16_t>> ReadCountersAtTimes(
    uint64_t first_cpu_time, uint64_t last_cpu_time, uint64_t cpu_time_spacing,
    const std::vector<const CounterInterface*>& counters) {
  std::vector<std::vector<uint16_t>> results;
  results.reserve((last_cpu_time - first_cpu_time) / cpu_time_spacing + 1);
  for (auto cpu_time = first_cpu_time; cpu_time <= last_cpu_time;
       cpu_time += cpu_time_spacing) {
    results.push_back(ReadCounters(cpu_time, counters));
  }
  return results;
}

void VisitCounterValuesAtTimes(
    uint64_t first_cpu_time, uint64_t last_cpu_time, uint64_t cpu_time_spacing,
    const std::vector<const CounterInterface*>& counters,
    std::function<void(const std::vector<uint16_t>& values)> visitor) {
  std::vector<uint16_t> values;
  values.reserve(counters.size());
  for (auto cpu_time = first_cpu_time; cpu_time <= last_cpu_time;
       cpu_time += cpu_time_spacing) {
    values.clear();  // Won't get rid of storage.
    for (const CounterInterface* counter : counters) {
      values.push_back(counter->ReadCounter(cpu_time));
    }
    visitor(values);
  }
}

std::vector<uint64_t> PickCpuTimeSamplesNear(uint64_t mean_cpu_time,
                                             uint64_t standard_deviation,
                                             int num_samples) {
  static std::random_device rd{};  // NOLINT
  static std::mt19937 gen{rd()};

  // LOG(INFO) << "mean=" << mean_cpu_time << " stddev=" << standard_deviation;

  std::normal_distribution<double> distribution(mean_cpu_time,
                                                standard_deviation);
  std::vector<uint64_t> values;
  values.reserve(num_samples);
  while (values.size() < num_samples) {
    double v = std::max(std::round(distribution(gen)), 0.0);
    values.push_back(static_cast<uint64_t>(v));
  }
  return values;
}

double EntropyOfDiscreteDistribution(std::vector<uint64_t> samples) {
  CHECK_GE(samples.size(), 1);
  std::sort(samples.begin(), samples.end());
  const double denominator = static_cast<double>(samples.size());
  double accumulator = 0;
  uint64_t current_sample = samples.front();
  int matching_samples = 1;
  for (size_t ndx = 1; ndx < samples.size(); ++ndx) {
    if (current_sample == samples[ndx]) {
      ++matching_samples;
      continue;
    } else {
      double probability = static_cast<double>(matching_samples) / denominator;
      double contribution = probability * std::log2(probability);
      // LOG(INFO) << matching_samples << " matching_samples / " << denominator
      //           << " -> " << probability << ", contribution " <<
      //           contribution;
      accumulator += contribution;
      current_sample = samples[ndx];
      matching_samples = 1;
    }
  }

  CHECK_GE(matching_samples, 1);

  double probability = static_cast<double>(matching_samples) / denominator;
  double contribution = probability * std::log2(probability);

  // LOG(INFO) << matching_samples << " matching_samples / " << denominator
  //           << " -> " << probability << ", contribution " << contribution;

  accumulator += contribution;
  return -accumulator;
}

double EntropyOfDiscreteDistribution(uint64_t mean_cpu_time,
                                     uint64_t standard_deviation,
                                     int num_samples) {
  return EntropyOfDiscreteDistribution(
      PickCpuTimeSamplesNear(mean_cpu_time, standard_deviation, num_samples));
}

uint32_t AvrRandom(uint32_t& context) {
  // The implementation in avr-libc/libc/stdlib/random.c appears to be roughly
  // equivalent to using this C++ type:
  //  std::linear_congruential_engine<std::uint32_t, 16807, 0, 2147483647>
  // But the details may be slightly different (esp. the handling of context
  // equal to 0), so I've ~copied the code in here from avr-libc. See that code
  // for the copyright notice.

  /*
   * Compute x = (7^5 * x) mod (2^31 - 1)
   * wihout overflowing 31 bits:
   *      (2^31 - 1) = 127773 * (7^5) + 2836
   * From "Random number generators: good ones are hard to find",
   * Park and Miller, Communications of the ACM, vol. 31, no. 10,
   * October 1988, p. 1195.
   */
  const uint32_t kRandomMax = 0x7fffffff;  // 2^31 - 1, a Mersenne Prime.
  uint32_t hi, lo, x = context;
  /* Can't be initialized with 0, so use another value. */
  if (x == 0) {
    x = 123459876L;
  }
  hi = x / 127773L;
  lo = x % 127773L;
  x = 16807L * lo - 2836L * hi;
  if (x < 0) {
    x += 0x7fffffffL;
  }
  return ((context = x) % (kRandomMax + 1));
}
