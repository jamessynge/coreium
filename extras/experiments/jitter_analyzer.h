#ifndef MCUCORE_EXTRAS_EXPERIMENTS_JITTER_ANALYZER_H_
#define MCUCORE_EXTRAS_EXPERIMENTS_JITTER_ANALYZER_H_

// This provides a simple model of AVR Timer/Counter (T/C) peripherals, for the
// purpose of answering this question:
//
//   If we arrange for multiple T/Cs to have different periods (e.g. primes),
//   and read their values every time a Watchdog timer (WDT) fires, how many WDT
//   interrupts do we need to read to ensure we have 'sufficient' randomness to
//   seed our pseudorandom number generator.
//
// 'Sufficient' here is focused on generating the MAC address, for which we want
// to pick 24 bits, though we also generate a link-local address, which requires
// another 16 bits.
//
// One way of exploring this is to say how many repeats of sampled values do we
// see in some interval. I've read a claim that reading the 8-bit T/C on 6 WDT
// and hashing the values produces a good 32-bit random number. My experiments
// with an Arduino UNO supported this. However, I'm interested in whether we can
// (unnecessarily?) shorten the time it takes to produce that seed. The ATmega
// WDT fires about every 16ms (+/- perhaps 10%). That does depend on how much
// jitter there is between the WDT and the T/C peripherals: if the WDT has lots
// of variation between devices AND temperature, then we don't need to worry
// much about sampling the counters at multiple WDT interrupts because we can
// treat each interrupt as a relatively independent source of random numbers
// (i.e. of the number of clock ticks between WDT interrupts).
//
// However, if the WDT is consistent between devices for a given temperature,
// and we can expect that many devices will be initialized at "room"
// temperature, there may not be as much variation in the values generated as we
// desire, and thus we might need to sample the counters multiple times.
//
// TODO(jamessynge): Write some code that measures how much variance there is in
// the firing of the WDT on several models of Arduino; for example, on each
// interrupt read the value of T/C#1 and store that in a ring buffer, then reset
// the counter to zero. Outside of the interrupt, print to Serial the counter
// values recorded in the ring buffer. On host record these and look to see how
// much variability we have in the different bits of the counter, and whether we
// see certain values more commonly than others. (I may have already done some
// of this during previous experiments... I really should publish the results.)
//
// NOTE: Regarding the different periods, these could be implemented directly as
// TOP of the counter, but could also be done with modulo of the counter,
// perhaps with TOP set to a multiple of the intended period (e.g. the largest
// multiple of the period that is not greater than the MAX value of the T/C).
//
////////////////////////////////////////////////////////////////////////////////
//
// An alternate perspective on how much randomness we can possibly get out of
// each WDT interrupt is to consider the number of Timer/Counter clock ticks
// that occur between the earliest time when the WDT interrupt might happen and
// the last time...
//
// For a typical Arduino with a 16MHz CPU clock and T/Cs with no prescaler (i.e.
// they run at the same speed as the CPU), with an average WDT interrupt spacing
// of 16ms, and interrupts occurring uniformly over a +/- 10% interval
// (i.e. 14.4ms to 17.6ms), there are only 51,200 clock ticks in that interval,
// which implies that we can produce at most about 15.6 bits on each clock tick.
// In other words, it doesn't matter whether our samples have more bits than
// that, we are still limited to at most log2(num_clock_ticks) unique values; we
// can of course generate fewer bits of randomness if we don't have that many
// bits of output.
//
// The assumption above of a uniform random distribution in the spacing of WDT
// interrupts seems highly suspect, and needs to be examined by performing
// experiments. If instead it is a gaussian distribution, with relatively small
// standard deviation, then we may produce only a few bits of randomness on each
// interrupt. Apparently the formula for the entropy (bits of information) of a
// random variable with Gaussian distribution is:
//
//    log2(2 * pi * e * variance) / 2
//
// However that is for a continuous random variable. Wikipedia says this about
// calculating Entropy (information theory):
//
//    In information theory, the entropy of a random variable is the average
//    level of "information", "surprise", or "uncertainty" inherent to the
//    variable's possible outcomes. Given a discrete random variable X, with
//    possible outcomes x1, ..., xn, which occur with probability P(x1), ...,
//    P(xn), the entropy of X is formally defined as:
//
//       H(X) = - (Sum for i=1 to n of (P(xi)*log2(P(xi)))

#include <cstdint>
#include <functional>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"

class CounterInterface {
 public:
  explicit CounterInterface(uint64_t starting_cpu_time,
                            uint16_t starting_clock_ticks, uint32_t prescaler)
      : starting_cpu_time_(starting_cpu_time),
        starting_clock_ticks_(starting_clock_ticks),
        prescaler_(prescaler) {
    // Prescaler must be a positive power of two.
    CHECK_GT(prescaler, 0);
    CHECK_EQ((prescaler & (prescaler - 1)), 0);
  }
  virtual ~CounterInterface() {}

  virtual uint16_t ReadCounter(uint64_t cpu_time) const = 0;

 protected:
  uint64_t ToClockTicks(uint64_t cpu_time) const {
    CHECK_GE(cpu_time, starting_cpu_time_);
    return (cpu_time - starting_cpu_time_) / prescaler_ + starting_clock_ticks_;
  }

  // Time at which the clock was started.
  const uint64_t starting_cpu_time_;

  // Initial value for the counter. We assume that the counter always starts
  // by counting up (there is an edge case: when the starting_clock_ticks_ is
  // the same as the maximum value of the counter, but we don't consider that
  // here).
  const uint16_t starting_clock_ticks_;

  // Linear adjustment (denominator) used to reduce the CPU ticks to the rate
  // desired for the clock ticks. Only a specific number of powers of 2 are
  // supported.
  const uint32_t prescaler_;
};

class SingleRampTimerCounter : public CounterInterface {
 public:
  SingleRampTimerCounter(uint64_t starting_cpu_time,
                         uint16_t starting_clock_ticks, uint32_t prescaler,
                         uint16_t maximum_ticks)
      : CounterInterface(starting_cpu_time, starting_clock_ticks, prescaler),
        maximum_ticks_(maximum_ticks) {}

  uint16_t ReadCounter(uint64_t cpu_time) const override {
    const auto clock_ticks = ToClockTicks(cpu_time);
    return clock_ticks % (maximum_ticks_ + 1);
  }

 private:
  const uint16_t maximum_ticks_;
};

class DualRampTimerCounter : public CounterInterface {
 public:
  DualRampTimerCounter(uint64_t starting_cpu_time,
                       uint16_t starting_clock_ticks, uint32_t prescaler,
                       uint16_t maximum_ticks)
      : CounterInterface(starting_cpu_time, starting_clock_ticks, prescaler),
        maximum_ticks_(maximum_ticks),
        cycle_ticks_(maximum_ticks_ * 2) {
    DVLOG(4) << "maximum_ticks=" << maximum_ticks
             << " cycle_ticks=" << cycle_ticks_;
  }

  uint16_t ReadCounter(uint64_t cpu_time) const override {
    const auto clock_ticks = ToClockTicks(cpu_time);
    const auto clock_ticks_in_cycle = clock_ticks % cycle_ticks_;
    DVLOG(4) << "cpu_time=" << cpu_time << " clock_ticks=" << clock_ticks
             << " clock_ticks_in_cycle=" << clock_ticks_in_cycle;
    if (clock_ticks_in_cycle <= maximum_ticks_) {
      return clock_ticks_in_cycle;
    } else {
      return maximum_ticks_ - (clock_ticks_in_cycle - maximum_ticks_);
    }
  }

 private:
  const uint16_t maximum_ticks_;
  const uint32_t cycle_ticks_;
};

// If 0, cpu_cycles_per_second defaults to the value expected for an Arduino
// Mega, which is 16,000,000.
uint64_t MillisToCpuTime(double ms, uint32_t cpu_cycles_per_second = 0);
double CpuTimeToMillis(uint64_t cpu_time, uint32_t cpu_cycles_per_second = 0);

std::vector<uint16_t> ReadCounters(
    uint64_t cpu_time, const std::vector<const CounterInterface*>& counters);

std::vector<std::vector<uint16_t>> ReadCountersAtTimes(
    uint64_t first_cpu_time, uint64_t last_cpu_time, uint64_t cpu_time_spacing,
    const std::vector<const CounterInterface*>& counters);

// Sort of like ReadCountersAtTimes, but avoids allocation for each sample.
void VisitCounterValuesAtTimes(
    uint64_t first_cpu_time, uint64_t last_cpu_time, uint64_t cpu_time_spacing,
    const std::vector<const CounterInterface*>& counters,
    std::function<void(const std::vector<uint16_t>& values)> visitor);

// These may not be unique, nor sorted.
std::vector<uint64_t> PickCpuTimeSamplesNear(uint64_t mean_cpu_time,
                                             uint64_t standard_deviation,
                                             int num_samples);

double EntropyOfDiscreteDistribution(std::vector<uint64_t> samples);

double EntropyOfDiscreteDistribution(uint64_t mean_cpu_time,
                                     uint64_t standard_deviation,
                                     int num_samples);

#endif  // MCUCORE_EXTRAS_EXPERIMENTS_JITTER_ANALYZER_H_
