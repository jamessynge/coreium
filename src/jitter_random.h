#ifndef MCUCORE_SRC_JITTER_RANDOM_H_
#define MCUCORE_SRC_JITTER_RANDOM_H_

// Provides a source of random integers, best used as a seed for a pseudo random
// number generator.
//
// This depends on the fact that the Watchdog Timer's clock isn't based on the
// same underlying clock as the CPU and the Timer/Counters. By reading the
// timer/counter registers every time the watchdog interrupt fires, we get a
// source of numbers that aren't consistent each time (i.e. there is some
// randomness); and by collecting values across multiple such interrupts, we get
// increasing amounts of unpredictable data. The implementation hashes the
// values we get from the timer/counter registers in order to be able spread the
// unpredictability across all of the bits of the 32-bits returned.
//
// Because the amount of randomness accumulated on each interrupt isn't very
// high, we need to allow time for the watchdog interrupt to occur multiple
// times. Earlier experiments (which should be repeated) found that we needed at
// least 6 interrupts to get 32-bits of randomness, but more interrupts will
// generally be better. Note though that the watchdog interrupts are fairly
// infrequent (e.g. ~16ms per interrupt on an ATmega2560), and are impacted by
// the supply voltage, Vcc, with lower voltages resulting in less frequent
// interrupts. Therefore it is best to use JitterRandom to provide a seed for
// the randomSeed() function of the Arduino core library. The default number of
// register reads was selected by recording lots of values produced by this
// function (see the jitter_random_iterations_tester.ino sketch), then assessing
// the randomness using the Chi-Squared test (see
// eval_jitter_random_iterations.py).
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"

namespace mcucore {

class JitterRandom {
 public:
  // Default minimum time over which to capture data for generating an random
  // number. 500ms should result in a pretty random number.
  static constexpr uint32_t kMinimumTimeMs = 500;

  // Default minimum number of watchdog interrupts to wait for, after
  // initializing the Timer/Counter peripherals.
  static constexpr uint8_t kMinimumWatchdogInterrupts = 20;

  enum ETimerCounterSelection {
    // Timer/Counter 0 is not actually 'taken over', but always used.
    kTimerCounter0 = 1 << 0,
    kTimerCounter1 = 1 << 1,
    kTimerCounter3 = 1 << 3,
    kTimerCounter4 = 1 << 4,
    kTimerCounter5 = 1 << 5,
    kTimerCounters01345 = kTimerCounter0 | kTimerCounter1 | kTimerCounter3 |
                          kTimerCounter4 | kTimerCounter5,
  };

  // Returns an unsigned 32-bit mostly random value, based on feeding hardware
  // clock jitter data into a hash function.
  //
  // `timer_counters_to_use` specifies which of the timer/counters to take over
  // for this purpose (i.e. which aren't currently being used for some other
  // purpose). We assume that Timer/Counter 0 is already configured by the
  // Arduino core libraries, and thus don't initialize it at the start, nor
  // disable it at the end. Regardless of the value of `timer_counters_to_use`,
  // all timer/counter registers will be read on each watchdog interrupt and
  // used as sources of unpredictable input, even though they all have the same
  // clock base.
  //
  // `minimum_watchdog_interrupts` the minimum number of calls to the watchdog
  // interrupt handler to wait for while accumulating input to the hash
  // function.
  //
  // `minimum_time_ms` is the minimum number of milliseconds over which to
  // accumulate input to the hash function.
  //
  // NOTE: This does not capture and later restore the state of the
  // Timer/Counter configuration registers, so take great care about the value
  // of `timer_counters_to_use`.
  static uint32_t random32(
      ETimerCounterSelection timer_counters_to_use = kTimerCounters01345,
      uint8_t minimum_watchdog_interrupts = kMinimumWatchdogInterrupts,
      uint32_t minimum_time_ms = kMinimumTimeMs);

  // Initialize the Arduino Random Number Generator with a seed produced by
  // random32(), with the specified arguments.
  static void setRandomSeed(
      ETimerCounterSelection timer_counters_to_use = kTimerCounters01345,
      uint8_t minimum_watchdog_interrupts = kMinimumWatchdogInterrupts,
      uint32_t minimum_time_ms = kMinimumTimeMs);
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_JITTER_RANDOM_H_
