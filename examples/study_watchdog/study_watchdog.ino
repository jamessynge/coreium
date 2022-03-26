// Exercise JitterRandom, to assess how well it is working "by eye."

#include <Arduino.h>
#include <McuCore.h>

using ::mcucore::BaseHex;
using ::mcucore::LogSink;

const auto tc0_at_global = TCNT0;
const auto tc1_at_global = TCNT1;
const auto tc2_at_global = TCNT2;
const auto tc3_at_global = TCNT3;
const auto tc4_at_global = TCNT4;
const auto tc5_at_global = TCNT5;
const auto millis_at_global = millis();
const auto micros_at_global = micros();

void setup() {
  const auto tc0 = TCNT0;
  const auto tc1 = TCNT1;
  const auto tc2 = TCNT2;
  const auto tc3 = TCNT3;
  const auto tc4 = TCNT4;
  const auto tc5 = TCNT5;

  const auto ms = millis();
  const auto us = micros();

  // Disable watchdog timer after reset; some AVR parts don't reset the watchdog
  // upon start, so the setting from a previous run can kill a future one. I'm
  // not sure if Arduino core for AVR already does this, in which case this
  // would be redundant.
  ::mcucore::avr::DisableWatchdog();

  // Setup serial with the fastest baud rate supported by the SoftwareSerial
  // class. Note that the baud rate is meaningful on boards with
  // microcontrollers that do 'true' serial (e.g. Arduino Uno and Mega), while
  // those boards with microcontrollers that have builtin USB (e.g. Arduino
  // Micro) likely don't rate limit because there isn't a need.
  Serial.begin(115200);

  // Wait for serial port to connect, or at least some minimum amount of time
  // (TBD), else the initial output gets lost. Note that this isn't true for all
  // Arduino-like boards: some reset when the Serial Monitor connects, so we
  // almost always get the initial output. Note though that a software reset
  // such as that may not reset all of the hardware features, leading to hard
  // to diagnose bugs (experience speaking).
  while (!Serial) {
  }
  LogSink() << MCU_FLASHSTR("\n\nSerial ready\n\n");
  LogSink() << MCU_FLASHSTR("Global values: ") << millis_at_global
            << MCU_FLASHSTR("ms, ") << micros_at_global << MCU_FLASHSTR("us")
            << BaseHex << ' ' << tc0_at_global << ' ' << tc1_at_global << ' '
            << tc2_at_global << ' ' << tc3_at_global << ' ' << tc4_at_global
            << ' ' << tc5_at_global;

  LogSink() << MCU_FLASHSTR("Local values: ") << ms << MCU_FLASHSTR("ms, ")
            << us << MCU_FLASHSTR("us") << BaseHex << ' ' << tc0 << ' ' << tc1
            << ' ' << tc2 << ' ' << tc3 << ' ' << tc4 << ' ' << tc5;

  LogSink() << MCU_FLASHSTR("\n\nStarting Watchdog Timer in Reset Mode");
  mcucore::avr::EnableWatchdogResetMode(4);  // ~0.25 seconds.
}

void loop() {
  Serial.print('.');
  delay(1);
}
