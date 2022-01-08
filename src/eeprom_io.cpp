#include "eeprom_io.h"

#include "hex_escape.h"
#include "logging.h"
#include "o_print_stream.h"
#include "progmem_string_data.h"

namespace mcucore {
namespace eeprom_io {

int SaveName(int toAddress, const char* name) {
  while (*name != 0) {
    EEPROM.put(toAddress++, *name++);
  }
  return toAddress;
}

int SaveName(int toAddress, const ProgmemStringView& name) {
  for (ProgmemStringView::size_type pos = 0; pos < name.size(); ++pos) {
    auto c = name.at(pos);
    EEPROM.put(toAddress++, c);
  }
  return toAddress;
}

bool VerifyName(int atAddress, const char* name, int* afterAddress) {
  // Confirm the name matches.
  while (*name != 0) {
    char c;
    EEPROM.get(atAddress++, c);
    if (c != *name++) {
      // Names don't match.
      return false;
    }
  }
  *afterAddress = atAddress;
  return true;
}

bool VerifyName(int atAddress, const ProgmemStringView& name,
                int* afterAddress) {
  // Confirm the name matches.
  for (ProgmemStringView::size_type pos = 0; pos < name.size(); ++pos) {
    const auto expected = name.at(pos);
    char c;
    EEPROM.get(atAddress++, c);
    if (c != expected) {
      // Names don't match.
      return false;
    }
  }
  *afterAddress = atAddress;
  return true;
}

void PutBytes(int address, const uint8_t* src, size_t numBytes, Crc32* crc) {
  while (numBytes-- > 0) {
    uint8_t b = *src++;
    if (crc) {
      crc->appendByte(b);
    }
    EEPROM.update(address++, b);
  }
}

void GetBytes(int address, size_t numBytes, uint8_t* dest, Crc32* crc) {
  while (numBytes-- > 0) {
    uint8_t b = EEPROM.read(address++);
    if (crc) {
      crc->appendByte(b);
    }
    *dest++ = b;
  }
}

int PutCrc(int toAddress, const Crc32& crc) {
  const auto value = crc.value();
  static_assert(4 == sizeof value, "sizeof CRC value is not 4");
  MCU_VLOG(6) << MCU_FLASHSTR("PutCrc value ") << BaseHex << value
              << MCU_FLASHSTR(" to ") << BaseDec << toAddress;
  EEPROM.put(toAddress, value);
  MCU_CHECK(VerifyCrc(toAddress, crc));
  return toAddress + static_cast<int>(sizeof value);
}

bool VerifyCrc(int atAddress, const Crc32& crc) {
  uint32_t stored = 0;
  EEPROM.get(atAddress, stored);
  if (crc.value() != stored) {
    MCU_VLOG(1) << MCU_FLASHSTR("VerifyCrc at ") << atAddress << BaseHex
                << MCU_FLASHSTR(") computed value=") << crc.value()
                << MCU_FLASHSTR(", stored value=") << BaseHex << stored;
    return false;
  }
  return true;
}

}  // namespace eeprom_io
}  // namespace mcucore
