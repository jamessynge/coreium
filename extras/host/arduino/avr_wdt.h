#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_AVR_WDT_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_AVR_WDT_H_

// A fake version of avr/wdt.h (Watchdog Timer Handling).

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

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_AVR_WDT_H_
