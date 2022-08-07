#include "McuCore.h"

// This is part of an experiment to see if we can log the value of the AVR MCUSR
// register when setup() is called by the Arduino infrastructure. Unfortunately
// it has already been modified by the time this code can run, so we can't
// determine whether the AVR MCU restarted for one of the reasons listed below.

void logMCUStatusRegister(uint8_t mcusr) {
  ::mcucore::LogSink() << MCU_PSD("MCUSR: ") << ::mcucore::BaseHex << mcusr;
  if (MCU_VLOG_IS_ON(1)) {
    if (mcusr & _BV(JTRF)) {
      // JTAG Reset
      MCU_VLOG(1) << MCU_PSD("JTAG") << MCU_PSD(" reset occured");
    }
    if (mcusr & _BV(WDRF)) {
      // Watchdog Reset
      MCU_VLOG(1) << MCU_PSD("Watchdog") << MCU_PSD(" reset occured");
    }
    if (mcusr & _BV(BORF)) {
      // Brownout Reset
      MCU_VLOG(1) << MCU_PSD("Brownout") << MCU_PSD(" reset occured");
    }
    if (mcusr & _BV(EXTRF)) {
      // Reset button or otherwise some software reset
      MCU_VLOG(1) << MCU_PSD("External") << MCU_PSD(" reset occured");
    }
    if (mcusr & _BV(PORF)) {
      // Power On Reset
      MCU_VLOG(1) << MCU_PSD("Power-on") << MCU_PSD(" reset occured");
    }
  }
}
