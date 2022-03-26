#ifndef MCUCORE_SRC_PLATFORM_AVR_WATCHDOG_H_
#define MCUCORE_SRC_PLATFORM_AVR_WATCHDOG_H_

#include "mcucore_platform.h"

namespace mcucore {
namespace avr {

// Disable the watchdog timer, for either interrupt or reset mode.
void DisableWatchdog();

// Enable the watchdog timer in interrupt mode (as opposed to reset mode), which
// will cause the WDT interrupt service route (aka ISR) to be called everytime
// the timeout determined by the prescaler is reached. The ISR should be
// declared as follows:
//
//     ISR(WDT_vect) { body; }
//
// Note that there can only be a single ISR(WDT_vect) declaration in the
// program. With the Arduino libraries, this can be worked around by using
// attachInterrupt to change the handler; this depends on the fact that AVR
// chips allow the interrupt handler vector to be in Flash or RAM, and Arduino
// provides the function attachInterrupt to allow overwriting the appropriate
// entry in the RAM vector.
//
// For the ATmega2560 and family, the prescaler is used in the following formula
// to determine the number of WDT oscillator cycles between interrupts:
//
//     2048 * (1 << prescaler)
//
// A prescaler of 0 produces a typical timeout of 16ms, given a Vcc of 5 volts.
// The oscillator has a frequency of approximately 128kHz, and is not tied to
// the CPU clock. The oscillator is not particularly precise, estimated to
// expire within about 10% of the nominal time period, which we exploit in
// JitterRandom). The prescaler is in the range 0 to 9, inclusive.
void EnableWatchdogInterruptMode(uint8_t prescaler = 0);

// Enable the watchdog timer in reset mode (as opposed to interrupt mode), which
// will cause the WDT to reset the microcontroller if ResetWatchdogCounter is
// not called more frequently than the watchdog timer period, which is
// determined by the prescaler. See the description above about the prescaler.
void EnableWatchdogResetMode(uint8_t prescaler = 0);

// Reset the watchdog timer (i.e. clear the watchdog timer's counter, which
// starts a new watchdog timer period). This is primarily used in reset mode,
// though it appears to be OK to use it in interrupt mode, too.
void ResetWatchdogCounter();

}  // namespace avr
}  // namespace mcucore

#endif  // MCUCORE_SRC_PLATFORM_AVR_WATCHDOG_H_
