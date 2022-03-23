#include "platform/avr/watchdog.h"

#include "logging.h"
#include "mcucore_platform.h"

// Not sure yet if I need to include <avr/wdt.h> to get the symbols needed here,
// or if including the Arduino headers might be sufficient.
#if MCU_HOST_TARGET
#include "extras/host/arduino/avr_wdt.h"  // pragma: keep extras include
#elif MCU_EMBEDDED_TARGET
#include <avr/wdt.h>
#endif

namespace mcucore {
namespace avr {
namespace {
constexpr uint8_t kMaxPrescaler = 9;
}

void EnableWatchdogInterrupts(uint8_t prescaler) {
  MCU_DCHECK_LE(prescaler, kMaxPrescaler) << prescaler;
  if (prescaler > kMaxPrescaler) {
    prescaler = kMaxPrescaler;
  }

  uint8_t wdt_configuration = (1 << WDIE);
  if ((prescaler & (1 << 3)) != 0) {
    wdt_configuration |= (1 << WDP3);
  }
  if ((prescaler & (1 << 2)) != 0) {
    wdt_configuration |= (1 << WDP2);
  }
  if ((prescaler & (1 << 1)) != 0) {
    wdt_configuration |= (1 << WDP1);
  }
  if ((prescaler & (1 << 0)) != 0) {
    wdt_configuration |= (1 << WDP0);
  }

  noInterrupts();

  // Clear WDRF (Watchdog Reset Flag) in MCUSR.
  MCUSR &= ~(1 << WDRF);

  // Enable changing the configuration of the watchdog.
  _WD_CONTROL_REG |= (1 << _WD_CHANGE_BIT) | (1 << WDE);

  // Set the watchdog (must be done within 4 clock cycles of enabling the
  // change).
  _WD_CONTROL_REG = wdt_configuration;

  interrupts();
}

void DisableWatchdogInterrupts() {
  noInterrupts();

  // Clear WDRF (Watchdog Reset Flag) in MCUSR.
  MCUSR &= ~(1 << WDRF);

  // Write logical one to WDCE and WDE; keep old prescaler setting to prevent
  // unintentional time-out.
  _WD_CONTROL_REG |= (1 << WDCE) | (1 << WDE);

  // Disable the watchdog.
  _WD_CONTROL_REG = 0x00;

  interrupts();
}

}  // namespace avr
}  // namespace mcucore
