// I experimented with the idea of using the watchdog timmer to help generate
// a random number here:
//
//    https://github.com/jamessynge/arduino_experiments
//
// See the jitter_* folders in that repo, and see jitter_random.* in the
// utilities folder of that repo for the implementation I used there. I am
// reimplementing here to see if I can get a benefit by using multiple
// Timer/Counter devices, even with the knowledge that they share a time base.
// In particular, can we arrange for them to 'appear' to drift apart, i.e. to
// remove the apparent correlation between them w.r.t. generating a random
// number. Clearly the values will still be correlated, but will that matter?
//
// TODO(jamessynge): Measure the effect of using FNV-1a vs. Crc32 as the means
// of hashing the values.
//
// TODO(jamessynge): Measure the effect of using the watchdog timer to separate
// the starting times of the Timer/Counter devices, vs. starting them with
// different initial values and different TOP values.
//
// NOTE: This is very specific to the ATmega2560, and would need to be modified
// to support other/multiple types of AVR processors, let along other processor
// families.

// #ifndef JITTER_RANDOM_ENABLE_MCU_VLOG
// #define MCU_DISABLE_VLOG
// #endif

#include "jitter_random.h"

#include "crc32.h"
#include "limits.h"
#include "logging.h"
#include "o_print_stream.h"
#include "platform/avr/watchdog.h"
#include "progmem_string_data.h"

// FNV-1a is purported to provided a better distribution of hash values,
// compared to CRC-32 which apparently has patterns (clumps) in the space of
// hash values.
#define USE_FNV1A  // Otherwise use CRC32

#ifdef USE_FNV1A

#define FNV1A_PRIME 0x01000193  // 2^24 + 2^8 + 0x93
#define FNV1A_INITIAL_VALUE 2166136261
#define FNV1A_BIT_SHIFT_INSTEAD_OF_MULTIPLY  // Measure whether this has any
                                             // benefit on the 8-bit AVR
                                             // processors.

#endif  // USE_FNV1A

namespace mcucore {
namespace {

volatile bool interrupted = false;

// Interrupt service routine, simply records that an interrupt occurred. We
// can get away with doing most of the work outside of the ISR because
// watchdog interrupts are far apart in time (e.g. around 15 milliseconds), so
// we have plenty of time to read the values of the registers and update the
// hashed value.
ISR(WDT_vect) {  // NOLINT
  interrupted = true;
}

void WaitForInterrupt() {
  while (interrupted == false) {
    // Wait for interrupt... except on a host target, where there aren't any.
#ifdef MCU_HOST_TARGET
    break;
#endif
  }
  interrupted = false;
}

// The timers are strongly correlated with each other, i.e. they have the same
// clock base. To slightly de-corrolate them, we separate reading from them
// by some amount based on the current crc value.
void SpinDelayBy(volatile uint16_t& loops) {
  while (loops > 0) {
    --loops;
  }
}

}  // namespace

// For accumulating state.
class JitterRandomCollector {
 public:
  explicit JitterRandomCollector(
      const JitterRandom::ETimerCounterSelection timer_counters_to_use)
      : timer_counters_to_use_(timer_counters_to_use) {
    // Enable counters. We don't need to enable Timer/Counter 0 because that
    // is done by the Arduino core for AVR, and is used as the basis for
    // millis() and micros().
    //
    // IDEAS FOR FURTHER DECORRELATION OF T/Cs:
    //
    // * Use a T/C mode with either OCRnA or ICRnA as TOP, and set each TOP to a
    //   different prime number (hard-coded), maybe with some of them just below
    //   256, and some of them just below 64K. The goal is that the spacing of
    //   the values from one T/C to another T/C that we read on one interrupt is
    //   changing on each interrupt. This is probably the best of these ideas.
    //
    // * Initialize each counter register to a different prime number,
    //   hard-coded; this may be unnecessary if we retain the WaitForInterrupt
    //   before each T/C is started.
    //
    // * Initialize every T/C as above, but don't start any of them before
    //   they're all initialized, then start them all in close succession. Not
    //   at all sure if this is a good idea.

#if MCU_HOST_TARGET || defined(TCNT1)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter1) {
      MCU_VLOG(3) << MCU_FLASHSTR("Initializing") << MCU_FLASHSTR(" T/C ") << 1;

      // We wait for a watchdog interrupt before starting the Timer/Counter;
      // this is an attempt to its correlation with the previously started T/Cs
      // by not having it always start at (approximately) the same number of
      // clock cycles after the previous one started.
      WaitForInterrupt();

      noInterrupts();
      TIMSK1 = 0;     // Disable interrupts from the Timer/Counter.
      TCCR1B = 0;     // Turn off the Timer/Counter.
      TCNT1 = 0;      // Clear the Timer/Counter register.
      TCCR1A = 0;     // Normal mode (not Compare Output Mode for any channel).
      TCCR1B = CS11;  // No prescaling of the clock, i.e. as fast as possible.
      interrupts();

      // Unless interrupted for a long time, there shouldn't have been time for
      // another watchdog interrupt.
      MCU_DCHECK(!interrupted);
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT3)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter3) {
      MCU_VLOG(3) << MCU_FLASHSTR("Initializing") << MCU_FLASHSTR(" T/C ") << 3;
      WaitForInterrupt();
      noInterrupts();
      TIMSK3 = 0;     // Disable interrupts from the Timer/Counter.
      TCCR3B = 0;     // Turn off the Timer/Counter.
      TCNT3 = 0;      // Clear the Timer/Counter register.
      TIMSK3 = 0;     // Disable interrupts from the Timer/Counter.
      TCCR3A = 0;     // Normal mode.
      TCCR3B = CS31;  // No prescaling of the clock.
      interrupts();
      MCU_DCHECK(!interrupted);
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT4)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter4) {
      MCU_VLOG(3) << MCU_FLASHSTR("Initializing") << MCU_FLASHSTR(" T/C ") << 4;
      WaitForInterrupt();
      noInterrupts();
      TIMSK4 = 0;     // Disable interrupts from the Timer/Counter.
      TCCR4B = 0;     // Turn off the Timer/Counter.
      TCNT4 = 0;      // Clear the Timer/Counter register.
      TIMSK4 = 0;     // Disable interrupts from the Timer/Counter.
      TCCR4A = 0;     // Normal mode.
      TCCR4B = CS41;  // No prescaling of the clock.
      interrupts();
      MCU_DCHECK(!interrupted);
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT5)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter5) {
      MCU_VLOG(3) << MCU_FLASHSTR("Initializing") << MCU_FLASHSTR(" T/C ") << 5;
      WaitForInterrupt();
      noInterrupts();
      TIMSK5 = 0;     // Disable interrupts from the Timer/Counter.
      TCCR5B = 0;     // Turn off the Timer/Counter.
      TCNT5 = 0;      // Clear the Timer/Counter register.
      TIMSK5 = 0;     // Disable interrupts from the Timer/Counter.
      TCCR5A = 0;     // Normal mode.
      TCCR5B = CS51;  // No prescaling of the clock.
      interrupts();
      MCU_DCHECK(!interrupted);
    }
#endif
  }

  ~JitterRandomCollector() {
    // Disable counters.

#if MCU_HOST_TARGET || defined(TCNT1)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter1) {
      MCU_VLOG(3) << MCU_FLASHSTR("Disabling") << MCU_FLASHSTR(" T/C ") << 1;
      noInterrupts();
      TCCR1B = 0;  // Turn off the Timer/Counter.
      interrupts();
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT3)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter3) {
      MCU_VLOG(3) << MCU_FLASHSTR("Disabling") << MCU_FLASHSTR(" T/C ") << 3;
      noInterrupts();
      TCCR3B = 0;  // Turn off the Timer/Counter.
      interrupts();
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT4)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter4) {
      MCU_VLOG(3) << MCU_FLASHSTR("Disabling") << MCU_FLASHSTR(" T/C ") << 4;
      noInterrupts();
      TCCR4B = 0;  // Turn off the Timer/Counter.
      interrupts();
    }
#endif

#if MCU_HOST_TARGET || defined(TCNT5)
    if (timer_counters_to_use_ & JitterRandom::kTimerCounter5) {
      MCU_VLOG(3) << MCU_FLASHSTR("Disabling") << MCU_FLASHSTR(" T/C ") << 5;
      noInterrupts();
      TCCR5B = 0;  // Turn off the Timer/Counter.
      interrupts();
    }
#endif
  }

  void CaptureCounters() {
    // Read each counter register, append to the crc, then wait a little bit
    // before reading the next register, where that little bit is a function
    // of the accumulated value so that it isn't the same every time.

    AppendByte(TCNT0);

#if MCU_HOST_TARGET || defined(TCNT1)
    {
      // Not sure if it makes any sense to add in a delay between reading T/Cs.
      volatile uint16_t loops = 701 + (value() % 739);
      SpinDelayBy(loops);
    }

    Append(TCNT1);
#endif

#if MCU_HOST_TARGET || defined(TCNT2)
    {
      volatile uint16_t loops = 709 + (value() % 743);
      SpinDelayBy(loops);
    }

    Append(TCNT2);
#endif

#if MCU_HOST_TARGET || defined(TCNT3)
    {
      volatile uint16_t loops = 719 + (value() % 751);
      SpinDelayBy(loops);
    }

    Append(TCNT3);
#endif

#if MCU_HOST_TARGET || defined(TCNT4)
    {
      volatile uint16_t loops = 727 + (value() % 757);
      SpinDelayBy(loops);
    }

    Append(TCNT4);
#endif

#if MCU_HOST_TARGET || defined(TCNT5)
    {
      volatile uint16_t loops = 733 + (value() % 761);
      SpinDelayBy(loops);
    }

    Append(TCNT5);
#endif
  }

  uint32_t value() const {
#ifdef USE_FNV1A
    return hash_;
#else
    return crc_.value();
#endif  // USE_FNV1A
  }

 private:
  void Append(uint16_t value) {
    AppendByte(static_cast<uint8_t>(value & 0xFF));
    AppendByte(static_cast<uint8_t>((value >> 8) & 0xFF));
  }

  void AppendByte(const uint8_t value) {
#ifdef USE_FNV1A
    // XOR the low order byte with value.
    MCU_DCHECK_EQ((hash_ ^ value), (hash_ ^ static_cast<uint32_t>(value)));
    hash_ ^= static_cast<uint32_t>(value);

    // Multiply by the 32 bit FNV magic prime, mod 2^32. The mod 2^32 is
    // required by the C++ (and C?) standard. See:
    //  https://en.cppreference.com/w/cpp/language/operator_arithmetic#Overflows
    // Here this means that while the intermediate result is (logically) a
    // uint64_t, it is converted to a uint32_t by retaining only the low 32 bits
    // of that intermediate result, and thus loses the information in the high
    // 32 bits.

#ifdef FNV1A_BIT_SHIFT_INSTEAD_OF_MULTIPLY
    // This could probably be further optimized for an 8-bit processor.
    hash_ += (hash_ << 1) + (hash_ << 4) + (hash_ << 7) + (hash_ << 8) +
             (hash_ << 24);
#else   // !FNV1A_BIT_SHIFT_INSTEAD_OF_MULTIPLY
    hash_ *= FNV1A_PRIME;
#endif  // FNV1A_BIT_SHIFT_INSTEAD_OF_MULTIPLY
#else   // !USE_FNV1A
    crc_.appendByte(value);
#endif  // USE_FNV1A

    MCU_VLOG(4) << MCU_FLASHSTR("AppendByte ") << BaseHex << value
                << MCU_FLASHSTR(", value is now ") << this->value();
  }

#ifdef USE_FNV1A
  uint32_t hash_ = FNV1A_INITIAL_VALUE;
#else   // !USE_FNV1A
  Crc32 crc_;
#endif  // USE_FNV1A
  const JitterRandom::ETimerCounterSelection timer_counters_to_use_;
};

uint32_t JitterRandom::random32(
    const JitterRandom::ETimerCounterSelection timer_counters_to_use,
    const uint8_t minimum_watchdog_interrupts, const uint32_t minimum_time_ms) {
  MCU_DCHECK(minimum_watchdog_interrupts > 0 || minimum_time_ms > 0);

  MCU_VLOG(1) << MCU_FLASHSTR("JitterRandom::random32 timer_counters_to_use: ")
              << BaseHex << static_cast<uint32_t>(timer_counters_to_use)
              << BaseDec << MCU_FLASHSTR(", minimum_watchdog_interrupts: ")
              << minimum_watchdog_interrupts;

  avr::EnableWatchdogInterruptMode();
  interrupted = false;

  JitterRandomCollector collector(timer_counters_to_use);
  uint16_t num_watchdog_interrupts = 0;
  uint32_t start_ms = millis();
  uint32_t elapsed_ms = 0;

  do {
    const auto copy_of_interrupted = interrupted;
    MCU_VLOG_IF(2, copy_of_interrupted)
        << MCU_FLASHSTR("loop start, already interrupted");
    MCU_VLOG(4) << MCU_NAME_VAL(num_watchdog_interrupts)
                << MCU_NAME_VAL(minimum_watchdog_interrupts)
                << MCU_NAME_VAL(elapsed_ms);

    WaitForInterrupt();
    MCU_VLOG(4) << MCU_FLASHSTR("interrupt detected");

    collector.CaptureCounters();

    if (num_watchdog_interrupts <
        numeric_limits<decltype(num_watchdog_interrupts)>::max()) {
      num_watchdog_interrupts++;
    }
    elapsed_ms = millis() - start_ms;
  } while (num_watchdog_interrupts < minimum_watchdog_interrupts ||
           elapsed_ms < minimum_time_ms);

  avr::DisableWatchdog();

  const auto result = collector.value();
  MCU_VLOG(1) << MCU_FLASHSTR("JitterRandom::random32") << MCU_NAME_VAL(result)
              << MCU_NAME_VAL(num_watchdog_interrupts)
              << MCU_NAME_VAL(elapsed_ms)
              << MCU_FLASHSTR(" elapsed_ms/interrupt=")
              << elapsed_ms / num_watchdog_interrupts;

  return result;
}

void JitterRandom::setRandomSeed(ETimerCounterSelection timer_counters_to_use,
                                 const uint8_t minimum_watchdog_interrupts,
                                 const uint32_t minimum_time_ms) {
  uint32_t seed = random32(timer_counters_to_use, minimum_watchdog_interrupts,
                           minimum_time_ms);
  MCU_VLOG(1) << MCU_FLASHSTR("JitterRandom::setRandomSeed to ") << BaseHex
              << seed;
  randomSeed(seed);
}

}  // namespace mcucore
