#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_AVR_WDT_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_AVR_WDT_H_

// A fake version of avr/wdt.h (Watchdog Timer Handling).
//
// Author: james.synge@gmail.com

#include <stdint.h>

#define wdt_reset() (void)0

#define WDTO_15MS 0
#define WDTO_30MS 1
#define WDTO_60MS 2
#define WDTO_120MS 3
#define WDTO_250MS 4
#define WDTO_500MS 5
#define WDTO_1S 6
#define WDTO_2S 7
#define WDTO_4S 8
#define WDTO_8S 9

inline void wdt_enable(uint8_t) {}

// These aren't in avr/wdt.h, but this is an appropriate place to define them
// for stubbing things out on host (i.e. a "do nothing" implementation, not an
// attempt to simulate the watchdog behavior).

#ifndef AVR_WDT_REGISTER_LINKAGE
#define AVR_WDT_REGISTER_LINKAGE extern
#endif  // !AVR_WDT_REGISTER_LINKAGE

#define WDP0 0
#define WDP1 1
#define WDP2 2
#define WDE 3
#define WDCE 4
#define WDP3 5
#define WDIE 6
#define WDIF 7

#define _WD_CHANGE_BIT WDCE

AVR_WDT_REGISTER_LINKAGE volatile uint8_t _WD_CONTROL_REG;  // NOLINT

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_AVR_WDT_H_
