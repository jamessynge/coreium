#ifndef MCUCORE_SRC_EEPROM_REGION_H_
#define MCUCORE_SRC_EEPROM_REGION_H_

// EepromRegionReader and EepromRegion represents a bounded region of an EEPROM.
// The classes are separate so that EepromTlv can return a reader for a block
// without requiring it to be writeable.

#include "logging.h"
#include "mcucore_platform.h"
#include "progmem_string_data.h"
#include "status_or.h"
#include "string_view.h"
#include "type_traits.h"

namespace mcucore {

// Supports reading from EEPROM.
class EepromRegionReader {
 public:
  // We don't support creating an EepromRegionReader of length 64KB.
  // Fortunately, all of the Microchip AVR chips that I know of have at most 4KB
  // of EEPROM.
  using AddrT = uint16_t;
  static constexpr AddrT kMaxAddrT = 65534;  // NOT 65535.
  using LengthT = uint16_t;
  static constexpr uint32_t kResourceExhausted =
      'F' + ('U' << 8) + ('L' << 16) + ('L' << 24);

  // Requires an EEPROMClass instance, rather than using the Arduino defined
  // EEPROM instance, so that testing is easier... NOT because we expect to work
  // with a device that has multiple EEPROMs!
  EepromRegionReader(EEPROMClass& eeprom, AddrT start_address, AddrT length)
      : eeprom_(&eeprom),
        start_address_(start_address),
        length_(length),
        cursor_(0) {
    static_assert(sizeof(LengthT) >= sizeof(decltype(eeprom.length())),
                  "EEPROM length can be too large.");
    MCU_DCHECK_LT(0, length);
    MCU_DCHECK_LE(length, kMaxAddrT);
    // Make sure that the region isn't too near the end of the space addressable
    // using an AddrT.
    MCU_DCHECK_LE(length - 1, kMaxAddrT - start_address)
        << MCU_FLASHSTR("Overflows region");  // COV_NF_LINE
  }

  explicit EepromRegionReader(EEPROMClass& eeprom, AddrT start_address = 0)
      : EepromRegionReader(eeprom, start_address,
                           eeprom.length() - start_address) {}

  EepromRegionReader(const EepromRegionReader&) = default;
  EepromRegionReader& operator=(const EepromRegionReader&) = default;

  AddrT start_address() const { return start_address_; }
  LengthT length() const { return length_; }

  // Cursor value is in the range [0, length()], inclusive.
  LengthT cursor() const { return cursor_; }
  LengthT available() const { return length_ - cursor_; }
  // Set the cursor to any value in the range [0, length()], inclusive. This
  // allows for setting the value to indicate that the region is "full".
  bool set_cursor(LengthT cursor) {
    if (cursor > length_) {
      return false;
    } else {
      cursor_ = cursor;
      return true;
    }
  }

  // Reads a value of type T from the EEPROM at the current cursor into the
  // specified output argument, advances the cursor and returns true, if the
  // value fits into the region; else returns false and does not advance the
  // cursor.
  template <typename T, enable_if_t<is_arithmetic<T>::value, bool> = true>
  bool ReadInto(T& output) {
    if (sizeof(T) > available()) {
      return false;
    }
    eeprom_->get(start_address_ + cursor_, output);
    cursor_ += sizeof(T);
    return true;
  }

  template <typename T, enable_if_t<is_arithmetic<T>::value, bool> = true>
  StatusOr<T> Read() {
    T value;
    if (ReadInto(value)) {
      return value;
    } else {
      return Status(kResourceExhausted);
    }
  }

  // Read `length` bytes from EEPROM starting at the cursor and write them to
  // the bytes starting at `ptr`, advances the cursor by `length` and returns
  // true, IFF there are at least `length` bytes available in the region
  // starting at the cursor; else returns false and does not advance the cursor.
  bool ReadBytes(uint8_t* ptr, const LengthT length) {
    if (length > available()) {
      return false;
    }
    AddrT from = start_address_ + cursor_;
    const AddrT beyond = from + length;
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
  bool ReadString(char* ptr, const LengthT length) {
    return ReadBytes(reinterpret_cast<uint8_t*>(ptr), length);
  }

 protected:
  EEPROMClass* eeprom_;

  // First address in the region.
  AddrT start_address_;
  // Size of the region.
  LengthT length_;

  // Where we'll next read from or write to *within* this block (i.e. it is an
  // offset from start_), unless otherwise specified. Every successful read or
  // write operation sets this to offset of the byte immediately after that
  // of the byte that was last read from or written to.
  LengthT cursor_;
};

// Supports writing to and reading from EEPROM.
class EepromRegion : public EepromRegionReader {
 public:
  EepromRegion(EEPROMClass& eeprom, AddrT start_address, AddrT length)
      : EepromRegionReader(eeprom, start_address, length) {}
  explicit EepromRegion(EEPROMClass& eeprom, AddrT start_address = 0)
      : EepromRegionReader(eeprom, start_address) {}
  EepromRegion(const EepromRegion&) = default;
  EepromRegion& operator=(const EepromRegion&) = default;

  // Writes `value` (a number or bool) to the EEPROM at the current cursor
  // location, advances the cursor by the length of T, and returns true, IFF
  // `value` fits in the region starting at cursor; else returns false and does
  // not advance the cursor.
  template <typename T, enable_if_t<is_arithmetic<T>::value, bool> = true>
  bool Write(const T value) {
    if (sizeof(T) > available()) {
      return false;
    }
    eeprom_->put(start_address_ + cursor_, value);
    cursor_ += sizeof(T);
    return true;
  }

  // Writes the `length` bytes starting at ptr to EEPROM, advances the cursor by
  // `length` and returns true, IFF the value fits in the region; else returns
  // false and does not advance the cursor.
  bool WriteBytes(const uint8_t* ptr, const LengthT length) {
    if (length > available()) {
      return false;
    }
    AddrT to = start_address_ + cursor_;
    const AddrT beyond = to + length;
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
  bool WriteString(const StringView& t) {
    return WriteBytes(reinterpret_cast<const uint8_t*>(t.data()), t.size());
  }
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_EEPROM_REGION_H_
