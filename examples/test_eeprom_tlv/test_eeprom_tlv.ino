#include <Arduino.h>
#include <McuCore.h>

using ::mcucore::EepromAddrT;
using ::mcucore::EepromRegion;
using ::mcucore::EepromRegionReader;
using ::mcucore::EepromTlv;
using ::mcucore::LogSink;
using ::mcucore::Status;

MCU_DEFINE_NAMED_DOMAIN(SOME_DATA, 99);

mcucore::EepromTag tag{MCU_DOMAIN(SOME_DATA), 8};

struct SomeData {
  Status Write(EepromRegion& region) const {
    if (!region.Write(f)) {
      return mcucore::UnknownError(MCU_PSV("write f failed"));
    }
    return mcucore::OkStatus();
  }

  Status Read(EepromRegionReader& reader) {
    if (!reader.ReadInto(f)) {
      return mcucore::UnknownError(MCU_PSV("read f failed"));
    }
    return mcucore::OkStatus();
  }

  double f;
};

Status WriteToRegion(EepromRegion& region, const SomeData& some_data) {
  return some_data.Write(region);
}

Status WriteToTlv(EepromTlv& tlv, const SomeData& some_data) {
  return tlv.WriteEntryToCursor(tag, sizeof(SomeData), WriteToRegion,
                                some_data);
}

// Status FillFromRegion(EepromRegion& region, SomeData& some_data) {
//   return some_data.Write(region);
// }

Status FillFromTlv(EepromTlv& tlv, SomeData& some_data) {
  MCU_ASSIGN_OR_RETURN(auto reader, tlv.FindEntry(tag));
  return some_data.Read(reader);
}

constexpr auto fs1 = MCU_PSV("fs1 with some value");

void setup() {
  // Setup serial, wait for it to be ready so that our logging messages can be
  // read.
  Serial.begin(115200);
  // Wait for serial port to connect, or at least some minimum amount of time
  // (TBD), else the initial output gets lost.
  while (!Serial) {
  }
  LogSink() << MCU_FLASHSTR("\nSerial ready")
            << MCU_FLASHSTR_128(
                   "\n########################################"
                   "########################################\n");

  LogSink() << MCU_FLASHSTR("Flash:");
  mcucore::HexDumpLeadingFlashBytes(Serial, 64);

  LogSink() << MCU_FLASHSTR("\nFlash string fs1:");
  mcucore::HexDumpFlashBytes(Serial, fs1);

  LogSink() << MCU_FLASHSTR("\nEEPROM:");
  mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  LogSink() << MCU_FLASHSTR("\nClearing start of EEPROM");
  for (size_t ndx = 0; ndx < 128; ++ndx) {
    EEPROM[ndx] = 0;
  }

  LogSink() << MCU_FLASHSTR("\nEEPROM:");
  mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  // LogSink() << MCU_FLASHSTR("EEPROM:");
  // mcucore::HexDumpFlashBytes(Serial, 0, 64);

  LogSink() << MCU_FLASHSTR("\nCopying fs1 to EEPROM using EepromRegion");
  {
    EepromRegion region(EEPROM, 0, 128);
    region.WriteString(fs1);
  }

  LogSink() << MCU_FLASHSTR("\nEEPROM:");
  mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  LogSink() << MCU_FLASHSTR("\nEepromTlv::ClearAndInitializeEeprom");
  MCU_CHECK_OK(EepromTlv::ClearAndInitializeEeprom(EEPROM));

  LogSink() << MCU_FLASHSTR("\nEEPROM:");
  mcucore::HexDumpEepromBytes(Serial, 0, 64, EEPROM);

  LogSink() << MCU_FLASHSTR("\nEepromTlv::GetIfValid");
  {
    auto status_or_tlv = EepromTlv::GetIfValid();
    if (status_or_tlv.ok()) {
      LogSink() << MCU_FLASHSTR("valid");
      auto tlv = status_or_tlv.value();
      MCU_CHECK_OK(tlv.Validate());
      LogSink() << tlv;

      SomeData some_data{.f = 0.0};
      WriteToTlv(tlv, some_data);
    } else {
      MCU_CHECK_OK(status_or_tlv);
    }
  }
}

void loop() {
  delay(2000);

  LogSink() << MCU_FLASHSTR_128(
                   "\n----------------------------------------"
                   "----------------------------------------\n")
            << MCU_FLASHSTR("EepromTlv::GetOrDie");
  auto tlv = EepromTlv::GetOrDie();
  LogSink() << MCU_FLASHSTR("got");
  MCU_CHECK_OK(tlv.Validate());
  LogSink() << MCU_FLASHSTR("validated");
  LogSink() << tlv << '\n';

  SomeData some_data{.f = 99999};
  MCU_CHECK_OK(FillFromTlv(tlv, some_data));
  LogSink() << MCU_FLASHSTR("found f=") << some_data.f;

  some_data.f += 1.01;
  LogSink() << MCU_FLASHSTR("writing f=") << some_data.f;
  MCU_CHECK_OK(WriteToTlv(tlv, some_data));
}
