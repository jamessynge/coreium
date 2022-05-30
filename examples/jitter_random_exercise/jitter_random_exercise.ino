// Exercise JitterRandom, to assess how well it is working "by eye."

#include <Arduino.h>
#include <McuCore.h>

using ::mcucore::BaseHex;
using ::mcucore::JitterRandom;
using ::mcucore::LogSink;
using ::mcucore::ProgmemString;

const auto boot_wdt_config = mcucore::avr::GetWatchdogConfig();

constexpr uint32_t kMinimumTimeMs = 100;
constexpr uint8_t kMinimumWatchdogInterrupts = 6;
constexpr JitterRandom::ETimerCounterSelection kTimerCounters =
    JitterRandom::kTimerCounters01345;
constexpr uint8_t kNumRandomsToGenerate = 5;

void setup() {
  

  // Disable watchdog timer after reset; some AVR parts don't reset the watchdog
  // upon start, so the setting from a previous run can kill a future one. I'm
  // not sure if Arduino core for AVR already does this, in which case this
  // would be redundant.
  mcucore::avr::DisableWatchdog();

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
  const auto serial_ready_us = micros();
  LogSink() //<< MCU_FLASHSTR("\n\nSerial ready") << MCU_NAME_VAL(serial_ready_us)
            << MCU_NAME_VAL(boot_wdt_config) << MCU_NAME_VAL(kMinimumTimeMs)
            << MCU_NAME_VAL(kMinimumWatchdogInterrupts) << BaseHex
            << MCU_NAME_VAL(kTimerCounters);

  for (uint8_t num_randoms = 0; num_randoms < kNumRandomsToGenerate;
       ++num_randoms) {  //
    const auto rnd = JitterRandom::random32(
        kTimerCounters, kMinimumWatchdogInterrupts, kMinimumTimeMs);
    LogSink() << MCU_FLASHSTR("#") << num_randoms << ' '// << mcucore::SetBase(36)
              << rnd;
  }

  // Reboot as soon as the watchdog fires.
  mcucore::avr::EnableWatchdogResetMode();
  while (true) {
    delay(1000);
  }
}

uint32_t Random32(const int counter_flags, const int interrupt_count) {
  return JitterRandom::random32(
      static_cast<JitterRandom::ETimerCounterSelection>(counter_flags),
      interrupt_count);
}

void Report(ProgmemString ps, const int counter_flags,
            const int interrupt_count) {
  const auto r = Random32(counter_flags, interrupt_count);
  LogSink() << ps << BaseHex << r;
}

void loop() {
  LogSink() << MCU_FLASHSTR("Loop Entry");

  for (int interrupt_count = 1; interrupt_count <= 6; ++interrupt_count) {
    const auto r0 = Random32(JitterRandom::kTimerCounter0, interrupt_count);
    const auto r01 = Random32(JitterRandom::kTimerCounter1, interrupt_count);
    const auto r013 =
        Random32(JitterRandom::kTimerCounter0 | JitterRandom::kTimerCounter1 |
                     JitterRandom::kTimerCounter3,
                 interrupt_count);
    const auto r0134 = Random32(
        JitterRandom::kTimerCounter0 | JitterRandom::kTimerCounter1 |
            JitterRandom::kTimerCounter3 | JitterRandom::kTimerCounter4,
        interrupt_count);
    const auto r01345 = Random32(
        JitterRandom::kTimerCounter0 | JitterRandom::kTimerCounter1 |
            JitterRandom::kTimerCounter3 | JitterRandom::kTimerCounter4 |
            JitterRandom::kTimerCounter5,
        interrupt_count);

#if 1  // Print in base 36.
    LogSink sink;
    sink << MCU_FLASHSTR("ic ") << interrupt_count << ' ';
    sink.set_base(36);
    sink << r0 << ' ' << r01 << ' ' << r013 << ' ' << r0134 << ' ' << r01345;
#else
    LogSink() << MCU_FLASHSTR("ic ") << interrupt_count << ' ' << r0 << ' '
              << r01 << ' ' << r013 << ' ' << r0134 << ' ' << r01345;
#endif

#if 0
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
#endif
  }
}
