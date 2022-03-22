// Exercise JitterRandom, to assess how well it is working "by eye."

#include <Arduino.h>
#include <McuCore.h>

using ::mcucore::BaseHex;
using ::mcucore::JitterRandom;
using ::mcucore::LogSink;
using ::mcucore::ProgmemString;

void setup() {
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
}

void Report(ProgmemString ps, int counter_flags) {
  const auto r = JitterRandom::random32(
      static_cast<JitterRandom::ETimerCounterSelection>(counter_flags));
  LogSink() << ps << BaseHex << r;
}

void loop() {
  LogSink() << MCU_FLASHSTR("Loop Entry");

  Report(MCU_FLASHSTR("T/C 0 only: "), JitterRandom::kTimerCounter0);

  Report(MCU_FLASHSTR("T/C 0, 1: "),
         JitterRandom::kTimerCounter0 | JitterRandom::kTimerCounter1);

  Report(MCU_FLASHSTR("T/C 0, 1, 3: "), JitterRandom::kTimerCounter0 |
                                            JitterRandom::kTimerCounter1 |
                                            JitterRandom::kTimerCounter3);

  Report(MCU_FLASHSTR("T/C 0, 1, 3, 4: "),
         JitterRandom::kTimerCounter0 | JitterRandom::kTimerCounter1 |
             JitterRandom::kTimerCounter3 | JitterRandom::kTimerCounter4);

  Report(MCU_FLASHSTR("T/C 0, 1, 3, 4, 5: "),
         JitterRandom::kTimerCounter0 | JitterRandom::kTimerCounter1 |
             JitterRandom::kTimerCounter3 | JitterRandom::kTimerCounter4 |
             JitterRandom::kTimerCounter5);

  delay(10 * 1000);
}
