#include "eeprom_region.h"

// TODO(jamessynge): Describe why this file exists/what it provides.

#include "flash_string_table.h"
#include "mcucore_platform.h"
#include "progmem_string_data.h"

namespace mcucore {

EepromRegionReader::EepromRegionReader(EEPROMClass& eeprom,
                                       const EepromAddrT start_address,
                                       const EepromAddrT length)
    : eeprom_(&eeprom),
      start_address_(start_address),
      length_(length),
      cursor_(0) {
  static_assert(is_same<EepromAddrT, decltype(eeprom.length())>::value);
  MCU_DCHECK_LE(start_address, eeprom.length())
      << MCU_FLASHSTR("Starts")           // COV_NF_LINE
      << MCU_FLASHSTR(" beyond EEPROM");  // COV_NF_LINE
  MCU_DCHECK_LE(length, eeprom.length() - start_address)
      << MCU_FLASHSTR("Extends")          // COV_NF_LINE
      << MCU_FLASHSTR(" beyond EEPROM");  // COV_NF_LINE
}

EepromRegionReader::EepromRegionReader(EEPROMClass& eeprom,
                                       EepromAddrT start_address)
    : EepromRegionReader(eeprom, start_address,
                         eeprom.length() - start_address) {}

EepromRegionReader::EepromRegionReader()
    : eeprom_(nullptr), start_address_(0), length_(0), cursor_(0) {}

bool EepromRegionReader::set_cursor(EepromAddrT cursor) {
  if (cursor > length_) {
    return false;
  } else {
    cursor_ = cursor;
    return true;
  }
}

void EepromRegionReader::Invalidate() {
  cursor_ = 0;
  length_ = 0;
}

bool EepromRegionReader::ReadBytes(uint8_t* ptr, const EepromAddrT length) {
  if (length > available()) {
    return false;
  }
  EepromAddrT from = start_address_ + cursor_;
  const EepromAddrT beyond = from + length;
  MCU_DCHECK_LE(from, beyond);
  while (from < beyond) {
    *ptr++ = eeprom_->read(from++);
  }
  MCU_DCHECK_EQ(from, beyond);
  if (from != beyond) {
    return false;  // COV_NF_LINE
  }
  cursor_ += length;
  return true;
}

// Reads `length` bytes from EEPROM into a char array. See ReadBytes for
// behavior details.
bool EepromRegionReader::ReadString(char* ptr, const EepromAddrT length) {
  return ReadBytes(reinterpret_cast<uint8_t*>(ptr), length);
}

// Prints fields, in support of googletest using OPrintStream and
// PrintValueToStdString.
void EepromRegionReader::InsertInto(OPrintStream& strm) const {
  strm << MCU_FLASHSTR("{.start=") << start_address_
       << MCU_FLASHSTR(", .length=") << length_ << MCU_FLASHSTR(", .cursor=")
       << cursor_ << MCU_FLASHSTR(", .available=") << available() << '}';
}

bool EepromRegion::WriteBytes(const uint8_t* ptr, const EepromAddrT length) {
  if (length > available()) {
    return false;
  }
  EepromAddrT to = start_address_ + cursor_;
  const EepromAddrT beyond = to + length;
  MCU_DCHECK_LE(to, beyond);
  while (to < beyond) {
    eeprom_->write(to++, *ptr++);
  }
  MCU_DCHECK_EQ(to, beyond);
  if (to != beyond) {
    return false;  // COV_NF_LINE
  }
  cursor_ += length;
  return true;
}

// Writes the characters of the string to EEPROM; see WriteBytes for behavior
// details. The caller is required to have some means of later determining the
// length of the string that was written in order to read it, such as writing
// the number of bytes in the string to EEPROM prior to writing the string's
// value, or writing a fixed size string.
bool EepromRegion::WriteString(const StringView& t) {
  return WriteBytes(reinterpret_cast<const uint8_t*>(t.data()), t.size());
}
}  // namespace mcucore
