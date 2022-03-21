#include "jitter_random.h"

// I experimented with the idea of using the watchdog timmer to help generate
// a random number here:
//
//    https://github.com/jamessynge/arduino_experiments
//
// See the jitter_* folders in that repo, and see jitter_random.* in the
// utilities folder of that repo for the implementation I used there. I am
// reimplementing here to see if I can get a benefit by using multiple
// timer/counter devices, even with the knowledge that they share a time base.
// In particular, can we arrange for them to 'appear' to drift apart, i.e. to
// remove the apparent correlation between them w.r.t. generating a random
// number. Clearly the values will still be correlated, but will that matter?
//
// TODO(jamessynge): Measure the effect of switching to Crc32 as the means of
// hashing the values.
//
// TODO(jamessynge): Measure the effect of using the watchdog timer to separate
// the starting times of the timer/counter devices, vs. starting them with
// different initial values and different TOP values.
//
// NOTE: This is very specific to the ATmega2560, and would need to be modified
// to support other/multiple types of AVR processors, let along other processor
// families.

#if MCU_HOST_TARGET
#include "extras/host/arduino/avr_wdt.h"  // pragma: keep extras include
#elif MCU_EMBEDDED_TARGET
#include <avr/wdt.h>
#endif

#include "crc32.h"
#include "logging.h"

namespace mcucore {
namespace {

volatile bool interrupted = false;

// Interrupt service routine, simply records that an interrupt occurred. We
// can get away with doing most of the work outside of the ISR because
// watchdog interrupts are far apart in time (e.g. around 15 milliseconds), so
// we have plenty of time to read the values of the registeres and update the
// Crc32.
ISR(WDT_vect) {  // NOLINT
  interrupted = true;
}

void WaitForInterrupt() {
  while (interrupted == false) {
    // Wait for interrupt.
  }
  interrupted = false;
}

void EnableWatchdogInterrupts() {
  noInterrupts();
  interrupted = false;
  MCUSR = 0;  // Is this needed?
  wdt_enable(WDTO_15MS);
  interrupts();
}

void DisableWatchdogInterrupts() {
  noInterrupts();
  // Paranoia: just in case some 'thread' is waiting for this to finish,
  // signal that an interrupt has occurred.
  interrupted = true;
  MCUSR = 0;  // Is this needed?
  wdt_reset();
  interrupts();
}

void WaitForOneInterrupt() {
  EnableWatchdogInterrupts();
  WaitForInterrupt();
  DisableWatchdogInterrupts();
}

}  // namespace

// For accumulating state.
class JitterRandomCollector {
 public:
  explicit JitterRandomCollector(
      const JitterRandom::ETimerCounterSelection timer_counters_to_use)
      : timer_counters_to_use_(timer_counters_to_use) {
    EnableWatchdogInterrupts();

    // Enable counters. We don't need to enable Timer/Counter 0 because that
    // is done by the Arduino core for AVR, and is used as the basis for
    // millis() and micros().

    // ALTERNATE IDEA FOR DECORRELATION: Before starting any T/C, initialize
    // each of their counter registers with a different prime number, and
    // initialize all of their control registers (except starting the clock).
    // And set them to use OCRnA or ICRnA as TOP, with a prime value of that
    // register, and to use different T/C modes for the different counters, to
    // the extend possible, so that we have different points where the counter
    // rolls over. Probably best to do that near 256, and to use only the low
    // 8-bits of the counter.

#if MCU_HOST_TARGET || defined(TCNT1)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter1) {
      noInterrupts();
      TIMSK1 = 0;  // Disable interrupts from the Timer/Counter.
      TCCR1B = 0;  // Turn off the Timer/Counter.
      TCNT1 = 0;   // Clear the Timer/Counter register.

      TCCR1A = 0;  // Normal mode (not Compare Output Mode for any channel).
      TCCR1B =
          CS11;  // No prescaling of the clock (i.e. run as fast as possible).
      interrupts();

      // Wait for a watchdog interrupt before starting another timer/counter.
      // This is an attempt to reduce their correlation by not having them all
      // start at nearly the same time, nor at an interval determined solely
      // by a fixed spacing due to the time it takes to execute the CPU
      // instructions.
      WaitForInterrupt();
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT3)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter3) {
      noInterrupts();
      TIMSK3 = 0;  // Disable interrupts from the Timer/Counter.
      TCCR3B = 0;  // Turn off the Timer/Counter.
      TCNT3 = 0;   // Clear the Timer/Counter register.
      TIMSK3 = 0;  // Disable interrupts from the Timer/Counter.

      TCCR3A = 0;  // Normal mode (not Compare Output Mode for any channel).
      TCCR3B =
          CS31;  // No prescaling of the clock (i.e. run as fast as possible).
      interrupts();
      WaitForInterrupt();
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT4)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter4) {
      noInterrupts();
      TIMSK4 = 0;  // Disable interrupts from the Timer/Counter.
      TCCR4B = 0;  // Turn off the Timer/Counter.
      TCNT4 = 0;   // Clear the Timer/Counter register.
      TIMSK4 = 0;  // Disable interrupts from the Timer/Counter.

      TCCR4A = 0;  // Normal mode (not Compare Output Mode for any channel).
      TCCR4B =
          CS41;  // No prescaling of the clock (i.e. run as fast as possible).
      interrupts();

      // Wait for a watchdog interrupt before starting another timer/counter.
      // This is an attempt to reduce their correlation by not having them all
      // start at nearly the same time, and at an interval determined solely
      // by CPU instructions, rather than by the watchdog or some external
      // source.
      WaitForInterrupt();
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT5)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter5) {
      noInterrupts();
      TIMSK5 = 0;  // Disable interrupts from the Timer/Counter.
      TCCR5B = 0;  // Turn off the Timer/Counter.
      TCNT5 = 0;   // Clear the Timer/Counter register.
      TIMSK5 = 0;  // Disable interrupts from the Timer/Counter.

      TCCR5A = 0;  // Normal mode (not Compare Output Mode for any channel).
      TCCR5B =
          CS51;  // No prescaling of the clock (i.e. run as fast as possible).
      interrupts();

      // Wait for a watchdog interrupt before starting another timer/counter.
      // This is an attempt to reduce their correlation by not having them all
      // start at nearly the same time, and at an interval determined solely
      // by CPU instructions, rather than by the watchdog or some external
      // source.
      WaitForInterrupt();
    }
#endif
  }

  ~JitterRandomCollector() {
    // Disable counters.
    noInterrupts();

#if MCU_HOST_TARGET || defined(TCNT1)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter1) {
      TCCR1B = 0;  // Turn off the Timer/Counter.
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT3)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter3) {
      TCCR3B = 0;  // Turn off the Timer/Counter.
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT4)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter4) {
      TCCR4B = 0;  // Turn off the Timer/Counter.
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT5)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter5) {
      TCCR5B = 0;  // Turn off the Timer/Counter.
    }
#endif

    interrupts();
  }

  uint32_t crc_value() { return crc_.value(); }

  void CaptureCounters() {
    // Read each counter register, append to the crc, then wait a little bit
    // before reading the next register, where that little bit is a function
    // of the accumulated value so that it isn't the same every time.
    crc_.appendByte(TCNT0);

#if MCU_HOST_TARGET || defined(TCNT1)
    {
      volatile uint16_t loops = 701 + (crc_.value() % 739);
      SpinDelayBy(loops);
    }

    Append(TCNT1);
#endif

#if MCU_HOST_TARGET || defined(TCNT2)
    {
      volatile uint16_t loops = 709 + (crc_.value() % 743);
      SpinDelayBy(loops);
    }

    Append(TCNT2);
#endif

#if MCU_HOST_TARGET || defined(TCNT3)
    {
      volatile uint16_t loops = 719 + (crc_.value() % 751);
      SpinDelayBy(loops);
    }

    Append(TCNT3);
#endif

#if MCU_HOST_TARGET || defined(TCNT4)
    {
      volatile uint16_t loops = 727 + (crc_.value() % 757);
      SpinDelayBy(loops);
    }

    Append(TCNT4);
#endif

#if MCU_HOST_TARGET || defined(TCNT5)
    {
      volatile uint16_t loops = 733 + (crc_.value() % 761);
      SpinDelayBy(loops);
    }

    Append(TCNT5);
#endif
  }

 private:
  // The timers are strongly correlated with each, i.e. they have the same
  // clock base. To slightly de-corrolate them, we separate reading from them
  // by some amount based on the current crc value.
  void SpinDelayBy(volatile uint16_t& loops) {
    while (loops > 0) {
      --loops;
    }
  }

  void Append(uint16_t value) {
    crc_.appendByte(static_cast<uint8_t>(value & 0xFF));
    crc_.appendByte(static_cast<uint8_t>((value >> 8) & 0xFF));
  }

  Crc32 crc_;

  const JitterRandom::ETimerCounterSelection timer_counters_to_use_;
};

uint32_t JitterRandom::random32(
    const JitterRandom::ETimerCounterSelection timer_counters_to_use,
    uint8_t num_watchdog_interrupts) {
  MCU_DCHECK_NE(num_watchdog_interrupts, 0);

  EnableWatchdogInterrupts();

  JitterRandomCollector collector(timer_counters_to_use);

  while (num_watchdog_interrupts > 0) {
    num_watchdog_interrupts--;
    WaitForOneInterrupt();
    collector.CaptureCounters();
  }

  DisableWatchdogInterrupts();

  return collector.crc_value();
}

}  // namespace mcucore
