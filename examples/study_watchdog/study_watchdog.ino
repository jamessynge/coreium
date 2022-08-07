// Exercise JitterRandom, to assess how well it is working "by eye."

#include <McuCore.h>

using ::mcucore::BaseDec;
using ::mcucore::BaseHex;
using ::mcucore::BaseTwo;
using ::mcucore::LogSink;
using ::mcucore::OPrintStream;

const auto SEP = MCU_FLASHSTR(", ");

// These don't capture all of the configuration, e.g. data direction config
// controls connection to I/O pins and interrupt configuration.

struct EightBitTimerCounterConfig {
  void InsertInto(OPrintStream& strm) const {
    strm << BaseTwo << MCU_PSD("TCCRnA=") << tccr_a << SEP  // Split
         << MCU_PSD("TCCRnB=") << tccr_b << SEP             // lines
         << MCU_PSD("OCRnA=") << ocr_a << SEP               // here.
         << MCU_PSD("OCRnB=") << ocr_b;
  }

  uint8_t tccr_a;
  uint8_t tccr_b;
  uint8_t ocr_a;
  uint8_t ocr_b;
};

struct TimerCounter0Config : EightBitTimerCounterConfig {
  TimerCounter0Config() {
    tccr_a = TCCR0A;
    tccr_b = TCCR0B;
    ocr_a = OCR0A;
    ocr_b = OCR0B;
  }

  void InsertInto(OPrintStream& strm) const {
    strm << MCU_PSD("T/C 0 Config: ");
    EightBitTimerCounterConfig::InsertInto(strm);
  }
};

struct TimerCounter2Config : EightBitTimerCounterConfig {
  TimerCounter2Config() {
    tccr_a = TCCR2A;
    tccr_b = TCCR2B;
    ocr_a = OCR2A;
    ocr_b = OCR2B;
    assr = ASSR;
  }

  void InsertInto(OPrintStream& strm) const {
    strm << MCU_PSD("T/C 2 Config: ");
    EightBitTimerCounterConfig::InsertInto(strm);
    strm << BaseTwo << SEP << MCU_PSD("ASSR=") << assr;
  }

  uint8_t assr;
};

struct SixteenBitTimerCounterConfig {
  explicit SixteenBitTimerCounterConfig(uint8_t tc_num) : tc_num(tc_num) {}

  void InsertInto(OPrintStream& strm) const {
    strm << MCU_PSD("T/C ") << BaseDec << tc_num  // Force
         << MCU_PSD(" Config: ") << BaseTwo       // formatter
         << MCU_PSD("TCCRnA=") << tccr_a << SEP   // to split
         << MCU_PSD("TCCRnB=") << tccr_b << SEP   // lines
         << MCU_PSD("TCCRnC=") << tccr_c << SEP   // here
         << MCU_PSD("OCRnA=") << ocr_a << SEP     // and
         << MCU_PSD("OCRnB=") << ocr_b << SEP     // here.
         << MCU_PSD("OCRnC=") << ocr_c;
  }

  const uint8_t tc_num;
  uint8_t tccr_a;
  uint8_t tccr_b;
  uint8_t tccr_c;
  uint16_t ocr_a;
  uint16_t ocr_b;
  uint16_t ocr_c;
};

struct TimerCounter1Config : SixteenBitTimerCounterConfig {
  TimerCounter1Config() : SixteenBitTimerCounterConfig(1) {
    tccr_a = TCCR1A;
    tccr_b = TCCR1B;
    tccr_c = TCCR1C;
    ocr_a = OCR1A;
    ocr_b = OCR1B;
    ocr_c = OCR1C;
  }
};
struct TimerCounter3Config : SixteenBitTimerCounterConfig {
  TimerCounter3Config() : SixteenBitTimerCounterConfig(3) {
    tccr_a = TCCR3A;
    tccr_b = TCCR3B;
    tccr_c = TCCR3C;
    ocr_a = OCR3A;
    ocr_b = OCR3B;
    ocr_c = OCR3C;
  }
};
struct TimerCounter4Config : SixteenBitTimerCounterConfig {
  TimerCounter4Config() : SixteenBitTimerCounterConfig(4) {
    tccr_a = TCCR4A;
    tccr_b = TCCR4B;
    tccr_c = TCCR4C;
    ocr_a = OCR4A;
    ocr_b = OCR4B;
    ocr_c = OCR4C;
  }
};
struct TimerCounter5Config : SixteenBitTimerCounterConfig {
  TimerCounter5Config() : SixteenBitTimerCounterConfig(5) {
    tccr_a = TCCR5A;
    tccr_b = TCCR5B;
    tccr_c = TCCR5C;
    ocr_a = OCR5A;
    ocr_b = OCR5B;
    ocr_c = OCR5C;
  }
};

struct TimerCapture {
  void InsertInto(OPrintStream& strm) const {
    strm << ms << MCU_PSD("ms") << SEP  // Split
         << us << MCU_PSD("us") << SEP  // lines
         << tc0 << SEP                  // here,
         << tc1 << SEP                  // here,
         << tc2 << SEP                  // here,
         << tc3 << SEP                  // and
         << tc4 << SEP                  // here.
         << tc5;
  }

  const uint8_t tc0 = TCNT0;
  const uint16_t tc1 = TCNT1;
  const uint8_t tc2 = TCNT2;
  const uint16_t tc3 = TCNT3;
  const uint16_t tc4 = TCNT4;
  const uint16_t tc5 = TCNT5;

  const uint32_t ms = millis();
  const uint32_t us = micros();
};

const auto global_watchdog_config = mcucore::avr::GetWatchdogConfig();
TimerCapture global_counters;
TimerCounter0Config global_tc0_config;
TimerCounter1Config global_tc1_config;
TimerCounter2Config global_tc2_config;
TimerCounter3Config global_tc3_config;
TimerCounter4Config global_tc4_config;
TimerCounter5Config global_tc5_config;

void setup() {
  TimerCapture setup_counters;

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
  const auto us = micros();

  LogSink() << MCU_PSD("\nSerial ready @ ") << us << MCU_PSD("us");
  LogSink() << MCU_PSD("Counter configs before init:\n");
  LogSink() << MCU_PSD("Watchdog Control Register: ") << BaseTwo
            << global_watchdog_config;
  LogSink() << global_tc0_config;
  LogSink() << global_tc1_config;
  LogSink() << global_tc2_config;
  LogSink() << global_tc3_config;
  LogSink() << global_tc4_config;
  LogSink() << global_tc5_config;

  LogSink() << MCU_PSD("\nCounter values before init: ") << global_counters;

  LogSink() << MCU_PSD("\nCounter configs in setup():");
  LogSink() << TimerCounter0Config();
  LogSink() << TimerCounter1Config();
  LogSink() << TimerCounter2Config();
  LogSink() << TimerCounter3Config();
  LogSink() << TimerCounter4Config();
  LogSink() << TimerCounter5Config();
  LogSink() << MCU_PSD("\nCounter values at setup() entry: ") << setup_counters;

  LogSink() << MCU_PSD("\n\nStarting Watchdog Timer in Reset Mode @ ")
            << micros() << MCU_PSD("us");
  mcucore::avr::EnableWatchdogResetMode(3);  // ~0.125 seconds.
  LogSink() << MCU_PSD("Watchdog Control Register: ") << BaseTwo
            << mcucore::avr::GetWatchdogConfig();
}

static bool first_loop = true;

void loop() {
  TimerCapture counters;
  if (first_loop || (counters.ms < 100 && (counters.ms % 10) == 0) ||
      (counters.ms >= 100 && (counters.ms % 100) == 0)) {
    LogSink() << counters;
    first_loop = false;
  }
}
