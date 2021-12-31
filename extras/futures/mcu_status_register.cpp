#include "McuCore.h"

// This is part of an experiment to see if we can log the value of the AVR MCUSR
// register when setup() is called by the Arduino infrastructure. Unfortunately
// it has already been modified by the time this code can run, so we can't
// determine whether the AVR MCU restarted for one of the reasons listed below.

void logMCUStatusRegister(uint8_t mcusr) {
  ::mcucore::LogSink() << MCU_FLASHSTR("MCUSR: ") << ::mcucore::BaseHex
                       << mcusr;
  if (MCU_VLOG_IS_ON(1)) {
    if (mcusr & _BV(JTRF)) {
      // JTAG Reset
      MCU_VLOG(1) << MCU_FLASHSTR("JTAG") << MCU_FLASHSTR(" reset occured");
    }
    if (mcusr & _BV(WDRF)) {
      // Watchdog Reset
      MCU_VLOG(1) << MCU_FLASHSTR("Watchdog") << MCU_FLASHSTR(" reset occured");
    }
    if (mcusr & _BV(BORF)) {
      // Brownout Reset
      MCU_VLOG(1) << MCU_FLASHSTR("Brownout") << MCU_FLASHSTR(" reset occured");
    }
    if (mcusr & _BV(EXTRF)) {
      // Reset button or otherwise some software reset
      MCU_VLOG(1) << MCU_FLASHSTR("External") << MCU_FLASHSTR(" reset occured");
    }
    if (mcusr & _BV(PORF)) {
      // Power On Reset
      MCU_VLOG(1) << MCU_FLASHSTR("Power-on") << MCU_FLASHSTR(" reset occured");
    }
  }
}
