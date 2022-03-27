#ifndef MCUCORE_SRC_EEPROM_REGION_H_
#define MCUCORE_SRC_EEPROM_REGION_H_

// EepromRegionReader and EepromRegion represents a bounded region of an EEPROM.
// The classes are separate so that EepromTlv can return a reader for a block
// without requiring it to be writeable.

#include "limits.h"
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
  // We need to represent lengths and addresses in the range:
  //          [0, eeprom.length()]
  // ... inclusive of the endpoints. Thus if EepromAddrT is uint16_t and the
  // length were 64KB, we wouldn't be able to represent the position the cursor
  // after we'd written the last byte.
  // As a result, we require that the length of the eeprom instance is less than
  // the maximum integer representable with EepromAddrT. Fortunately, all of the
  // Microchip AVR chips that I know of have at most 4KB of EEPROM.
  static constexpr EepromAddrT kMaxAddrT =
      numeric_limits<EepromAddrT>::max() - 1;

  // Requires an EEPROMClass instance, rather than implicitly using the Arduino
  // defined EEPROM instance; this makes testing is easier, and is NOT because
  // we expect to work with a device that has multiple EEPROMs! We allow the
  // length to be zero to allow for marking a region as unusable.
  EepromRegionReader(EEPROMClass& eeprom, const EepromAddrT start_address,
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

  explicit EepromRegionReader(EEPROMClass& eeprom,
                              EepromAddrT start_address = 0)
      : EepromRegionReader(eeprom, start_address,
                           eeprom.length() - start_address) {}

  // An empty, unusable region. Can be made usable be assignment.
  EepromRegionReader()
      : eeprom_(nullptr), start_address_(0), length_(0), cursor_(0) {}

  EepromRegionReader(const EepromRegionReader&) = default;
  EepromRegionReader& operator=(const EepromRegionReader&) = default;

  EepromAddrT start_address() const { return start_address_; }
  EepromAddrT length() const { return length_; }

  // Cursor value is in the range [0, length()], inclusive.
  EepromAddrT cursor() const { return cursor_; }
  EepromAddrT available() const { return length_ - cursor_; }
  // Set the cursor to any value in the range [0, length()], inclusive. This
  // allows for setting the value to indicate that the region is "full".
  bool set_cursor(EepromAddrT cursor) {
    if (cursor > length_) {
      return false;
    } else {
      cursor_ = cursor;
      return true;
    }
  }

  // Modify this instances so that it can no longer be used for reading or
  // writing.
  void Invalidate() {
    cursor_ = 0;
    length_ = 0;
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
      return Status(StatusCode::kResourceExhausted);
    }
  }

  // Read `length` bytes from EEPROM starting at the cursor and write them to
  // the bytes starting at `ptr`, advances the cursor by `length` and returns
  // true, IFF there are at least `length` bytes available in the region
  // starting at the cursor; else returns false and does not advance the cursor.
  bool ReadBytes(uint8_t* ptr, const EepromAddrT length) {
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
  bool ReadString(char* ptr, const EepromAddrT length) {
    return ReadBytes(reinterpret_cast<uint8_t*>(ptr), length);
  }

 protected:
  EEPROMClass* eeprom_;

  // First address in the region.
  EepromAddrT start_address_;
  // Size of the region.
  EepromAddrT length_;

  // Where we'll next read from or write to *within* this block (i.e. it is an
  // offset from start_), unless otherwise specified. Every successful read or
  // write operation sets this to offset of the byte immediately after that
  // of the byte that was last read from or written to.
  EepromAddrT cursor_;
};

// Supports writing to and reading from EEPROM.
class EepromRegion : public EepromRegionReader {
 public:
  EepromRegion(EEPROMClass& eeprom, EepromAddrT start_address,
               EepromAddrT length)
      : EepromRegionReader(eeprom, start_address, length) {}
  explicit EepromRegion(EEPROMClass& eeprom, EepromAddrT start_address = 0)
      : EepromRegionReader(eeprom, start_address) {}
  EepromRegion() : EepromRegionReader() {}
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
  bool WriteBytes(const uint8_t* ptr, const EepromAddrT length) {
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
  bool WriteString(const StringView& t) {
    return WriteBytes(reinterpret_cast<const uint8_t*>(t.data()), t.size());
  }
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_EEPROM_REGION_H_
