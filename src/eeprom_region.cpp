#include "eeprom_region.h"

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
      << MCU_PSD("Starts")           // COV_NF_LINE
      << MCU_PSD(" beyond EEPROM");  // COV_NF_LINE
  MCU_DCHECK_LE(length, eeprom.length() - start_address)
      << MCU_PSD("Extends")          // COV_NF_LINE
      << MCU_PSD(" beyond EEPROM");  // COV_NF_LINE
}

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
  strm << MCU_PSD("{.start=") << start_address_ << MCU_PSD(", .length=")
       << length_ << MCU_PSD(", .cursor=") << cursor_
       << MCU_PSD(", .available=") << available() << '}';
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

bool EepromRegion::WriteString(const StringView& t) {
  return WriteBytes(reinterpret_cast<const uint8_t*>(t.data()), t.size());
}

bool EepromRegion::WriteString(const ProgmemStringView psv) {
  const auto size = psv.size();
  if (size > available()) {
    return false;
  }
  EepromAddrT to = start_address_ + cursor_;
  for (ProgmemStringView::size_type ndx = 0; ndx < size; ++ndx) {
    char c = psv.at(ndx);
    eeprom_->write(to++, c);
  }
  cursor_ += size;
  return true;
}

}  // namespace mcucore
