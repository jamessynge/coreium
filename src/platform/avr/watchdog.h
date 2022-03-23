#ifndef MCUCORE_SRC_PLATFORM_AVR_WATCHDOG_H_
#define MCUCORE_SRC_PLATFORM_AVR_WATCHDOG_H_

#include "mcucore_platform.h"

namespace mcucore {
namespace avr {

// Enable the watchdog timer in interrupt mode (as opposed to reset mode), which
// will cause the WDT interrupt service route (aka ISR) to be called everytime
// the timeout determined by the prescaler is reached. The ISR should be
// declared as follows:
//
//     ISR(WDT_vect) { body; }
//
// Note that there can only be a single ISR(WDT_vect) declaration in the
// program.
//
// For the ATmega2560 and family, the prescaler is used in the following formula
// to determine the number of WDT oscillator cycles between interrupts:
//
//     2048 * (1 << prescaler)
//
// A prescaler of 0 produces a typical timeout of 16ms, given a Vcc of 5 volts.
// The oscillator has a frequency of approximately 128kHz, but it is not
// particularly precise, nor is it tied to the CPU clock (which we can exploit
// in JitterRandom). The prescaler is in the range 0 to 9, inclusive.
void EnableWatchdogInterrupts(uint8_t prescaler = 0);

// Disable the watchdog timer.
void DisableWatchdogInterrupts();

}  // namespace avr
}  // namespace mcucore

#endif  // MCUCORE_SRC_PLATFORM_AVR_WATCHDOG_H_
