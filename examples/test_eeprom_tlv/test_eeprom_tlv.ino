#include <Arduino.h>
#include <McuCore.h>

using ::mcucore::EepromAddrT;
using ::mcucore::EepromRegion;
using ::mcucore::EepromTlv;
using ::mcucore::LogSink;
using ::mcucore::StringView;

constexpr auto fs1 = MCU_PSV("fs1 with some value");
constexpr auto fs2 = MCU_PSV("fs2 with another value");
constexpr auto fs3 = MCU_PSV("fs1 with some other value");

void setup() {
  // Setup serial, wait for it to be ready so that our logging messages can be
  // read.
  Serial.begin(115200);
  // Wait for serial port to connect, or at least some minimum amount of time
  // (TBD), else the initial output gets lost.
  while (!Serial) {
  }
  mcucore::LogSink() << MCU_FLASHSTR("Serial ready");
  mcucore::LogSink() << MCU_FLASHSTR_128(
      "####################################################################");
  mcucore::LogSink();

  mcucore::LogSink() << MCU_FLASHSTR("Flash:");
  mcucore::HexDumpLeadingFlashBytes(Serial, 64);

  mcucore::LogSink();
  mcucore::LogSink() << MCU_FLASHSTR("Flash string fs1:");
  mcucore::HexDumpFlashBytes(Serial, fs1);

  mcucore::LogSink();
  mcucore::LogSink() << MCU_FLASHSTR("EEPROM:");
  mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  mcucore::LogSink();
  mcucore::LogSink() << MCU_FLASHSTR("Clearing start of EEPROM");
  for (size_t ndx = 0; ndx < 128; ++ndx) {
    EEPROM[ndx] = 0;
  }

  mcucore::LogSink();
  mcucore::LogSink() << MCU_FLASHSTR("EEPROM:");
  mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  // mcucore::LogSink() << MCU_FLASHSTR("EEPROM:");
  // mcucore::HexDumpFlashBytes(Serial, 0, 64);

  mcucore::LogSink();
  mcucore::LogSink() << MCU_FLASHSTR(
      "Copying fs1 to EEPROM using EepromRegion");
  {
    EepromRegion region(EEPROM, 0, 128);
    region.WriteString(fs1);
  }

  mcucore::LogSink();
  mcucore::LogSink() << MCU_FLASHSTR("EEPROM:");
  mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  mcucore::LogSink();
  mcucore::LogSink() << MCU_FLASHSTR("EepromTlv::ClearAndInitializeEeprom");
  MCU_CHECK_OK(EepromTlv::ClearAndInitializeEeprom(EEPROM));

  mcucore::LogSink();
  mcucore::LogSink() << MCU_FLASHSTR("EEPROM:");
  mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  // mcucore::LogSink() << MCU_FLASHSTR("EEPROM:");
  // mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  // mcucore
}

void loop() {  //
  delay(10000);
}
