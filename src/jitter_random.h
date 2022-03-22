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

#include "mcucore_platform.h"

namespace mcucore {

class JitterRandom {
 public:
  enum ETimerCounterSelection {
    // Timer/Counter 0 is not actually 'taken over', but always used.
    kTimerCounter0 = 1 << 0,
    kTimerCounter1 = 1 << 1,
    kTimerCounter3 = 1 << 3,
    kTimerCounter4 = 1 << 4,
    kTimerCounter5 = 1 << 5,
    kTimerCounters1345 =
        kTimerCounter1 | kTimerCounter3 | kTimerCounter4 | kTimerCounter5,
  };

  // Returns an unsigned 32-bit pseudo random value. `timer_counters_to_use`
  // specifies which of the timer/counters to take over for this purpose (i.e.
  // which aren't currently being used for some other purpose).
  // `num_watchdog_interrupts` specifies how many calls to the watchdog
  // interrupt handler to wait for; must be non-zero.
  //
  // We assume that Timer/Counter 0 is already configured by the Arduino core
  // libraries, and read from its count when reading the counters.
  //
  // NOTE: This does not capture and later restore the state of the
  // Timer/Counter configuration registers, so take great care about the value
  // of `timer_counters_to_use`.
  //
  // Regardless of the value of `timer_counters_to_use`, all timer/counter
  // registers will be read on each watchdog interrupt and used as sources of
  // unpredictable input, even though they all have the same clock base.
  static uint32_t random32(ETimerCounterSelection timer_counters_to_use,
                           uint8_t num_watchdog_interrupts = 15);
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_JITTER_RANDOM_H_
