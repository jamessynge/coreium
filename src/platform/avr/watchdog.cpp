// This implementation is specific to ATmega640/1280/1281/2560/2561, and has not
// been generalized or tested with other AVR models. From the datasheet for
// those models:
//
//    The WDT gives an interrupt or a system reset when the counter reaches a
//    given time-out value. In normal operation mode, it is required that the
//    system uses the WDR - Watchdog Timer Reset - instruction to restart the
//    counter before the time-out value is reached. If the system doesn't
//    use the WDR - Watchdog Timer Reset - instruction to restart the
//    counter, an interrupt or system reset will be issued.
//
//    ... alterations to the Watchdog set-up must follow timed sequences. The
//    sequence for clearing WDE and changing time-out configuration is as
//    follows:
//    1. In the same operation, write a logic one to the Watchdog change enable
//       bit (WDCE) and WDE. A logic one must be written to WDE regardless of
//       the previous value of the WDE bit.
//    2. Within the next four clock cycles, write the WDE and Watchdog prescaler
//       bits (WDP) as desired, but with the WDCE bit cleared. This must be done
//       in one operation.
//    ...
//    To clear WDE (Watchdog System Reset Enable), WDRF (Watchdog Reset Flag)
//    must be cleared first. This feature ensures multiple resets during
//    conditions causing failure, and a safe start-up after the failure.

#include "platform/avr/watchdog.h"

#include "mcucore_platform.h"
#include "strings/progmem_string_data.h"

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

void SetControlRegister(uint8_t new_value) {
  noInterrupts();

  // Clear WDRF (Watchdog Reset Flag) in MCUSR. This is required in order to
  // clear the WDE flag (which we may be doing).
  MCUSR &= ~(1 << WDRF);

  // Write logical one to WDCE (Change Enable) and WDE bits. This enables
  // changing the configuration of the watchdog.
  _WD_CONTROL_REG |= (1 << WDCE) | (1 << WDE);

  // Overwrite the register.
  _WD_CONTROL_REG = new_value;

  interrupts();
}

constexpr uint8_t kMaxPrescaler = 9;  // 0b1001

uint8_t PrescalerToRegisterMask(uint8_t prescaler) {
#ifdef MCU_ENABLE_DCHECK
  // We can't include mcucore/src/logging.h here, else we'll create a dependency
  // cycle, so we print directly to ::Serial if DCHECK is allowed.
  if (prescaler > kMaxPrescaler) {
    Serial.print(MCU_FLASHSTR("AVR WDT Prescaler out of range: "));
    Serial.print(prescaler + 0);
    Serial.print('>');
    Serial.println(kMaxPrescaler + 0);
    delay(1000);
    // Effectively call EnableWatchdogResetMode(0), but without risking
    // recursion into this method.
    SetControlRegister(1 << WDE);
    while (true) {
      // A deliberately infinite loop.
      delay(1000);
    }
  }
#endif
  if (prescaler > kMaxPrescaler) {
    prescaler = kMaxPrescaler;
  }
  uint8_t mask = 0;
  if ((prescaler & (1 << 3)) != 0) {
    mask |= (1 << WDP3);
  }
  if ((prescaler & (1 << 2)) != 0) {
    mask |= (1 << WDP2);
  }
  if ((prescaler & (1 << 1)) != 0) {
    mask |= (1 << WDP1);
  }
  if ((prescaler & (1 << 0)) != 0) {
    mask |= (1 << WDP0);
  }
  return mask;
}

}  // namespace

void DisableWatchdog() { SetControlRegister(0x00); }

void EnableWatchdogInterruptMode(uint8_t prescaler) {
  SetControlRegister((1 << WDIE) | PrescalerToRegisterMask(prescaler));
}

void EnableWatchdogResetMode(uint8_t prescaler) {
  SetControlRegister((1 << WDE) | PrescalerToRegisterMask(prescaler));
}

void ResetWatchdogCounter() { wdt_reset(); }

uint8_t GetWatchdogConfig() { return _WD_CONTROL_REG; }

}  // namespace avr
}  // namespace mcucore
