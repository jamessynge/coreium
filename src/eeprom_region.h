#ifndef MCUCORE_SRC_EEPROM_REGION_H_
#define MCUCORE_SRC_EEPROM_REGION_H_

// EepromRegion represents a bounded region of an EEPROM.

#include "logging.h"
#include "mcucore_platform.h"
#include "progmem_string_data.h"
#include "status_or.h"
#include "string_view.h"
#include "type_traits.h"

namespace mcucore {

class EepromRegion {
 public:
  // We don't support creating an EepromRegion of size 64KB. Fortunately, all
  // of the Microchip AVR chips that I know of have at most 4KB of EEPROM.
  using AddrT = uint16_t;
  static constexpr AddrT kMaxAddrT = 65534;  // NOT 65535.
  using SizeT = uint16_t;
  static constexpr uint32_t kResourceExhausted =
      'F' + ('U' << 8) + ('L' << 16) + ('L' << 24);

  EepromRegion(AddrT start_address, AddrT size)
      : start_address_(start_address), size_(size), cursor_(0) {
    MCU_DCHECK_LT(0, size);
    MCU_DCHECK_LE(size, kMaxAddrT);
    // Make sure that the region isn't too near the end of the space addressable
    // using an AddrT.
    MCU_DCHECK_LE(size - 1, kMaxAddrT - start_address)
        << MCU_FLASHSTR("Overflows region");  // COV_NF_LINE
  }

  explicit EepromRegion(AddrT start_address)
      : EepromRegion(start_address, EEPROM.length() - start_address) {}

  EepromRegion() : EepromRegion(0, EEPROM.length()) {
    static_assert(sizeof(SizeT) >= sizeof(decltype(EEPROM.length())),
                  "EEPROM size can be too large.");
  }

  AddrT start_address() const { return start_address_; }
  SizeT size() const { return size_; }

  // Cursor value is in the range [0, size()], inclusive.
  SizeT cursor() const { return cursor_; }
  SizeT available() const { return size_ - cursor_; }
  // Set the cursor to any value in the range [0, size()], inclusive. This
  // allows for setting the value to indicate that the region is "full".
  bool set_cursor(SizeT cursor) {
    if (cursor > size_) {
      return false;
    } else {
      cursor_ = cursor;
      return true;
    }
  }

  // Writes `value` (a number or bool) to the EEPROM at the current cursor
  // location, advances the cursor by the size of T, and returns true, IFF
  // `value` fits in the region starting at cursor; else returns false and does
  // not advance the cursor.
  template <typename T, enable_if_t<is_arithmetic<T>::value, bool> = true>
  bool Write(const T value) {
    if (sizeof(T) > available()) {
      return false;
    }
    EEPROM.put(start_address_ + cursor_, value);
    cursor_ += sizeof(T);
    return true;
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
    EEPROM.get(start_address_ + cursor_, output);
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

  // Writes the `size` bytes starting at ptr to EEPROM, advances the cursor by
  // `size` and returns true, IFF the value fits in the region; else returns
  // false and does not advance the cursor.
  bool WriteBytes(const uint8_t* ptr, const SizeT size) {
    if (size > available()) {
      return false;
    }
    AddrT to = start_address_ + cursor_;
    const AddrT beyond = to + size;
    MCU_DCHECK_LE(to, beyond);
    while (to < beyond) {
      EEPROM.write(to++, *ptr++);
    }
    MCU_DCHECK_EQ(to, beyond);
    if (to != beyond) {
      return false;  // COV_NF_LINE
    }
    cursor_ += size;
    return true;
  }

  // Read `size` bytes from EEPROM starting at the cursor and write them to the
  // bytes starting at `ptr`, advances the cursor by `size` and returns true,
  // IFF there are at least `size` bytes available in the region starting at the
  // cursor; else returns false and does not advance the cursor.
  bool ReadBytes(uint8_t* ptr, const SizeT size) {
    if (size > available()) {
      return false;
    }
    AddrT from = start_address_ + cursor_;
    const AddrT beyond = from + size;
    MCU_DCHECK_LE(from, beyond);
    while (from < beyond) {
      *ptr++ = EEPROM.read(from++);
    }
    MCU_DCHECK_EQ(from, beyond);
    if (from != beyond) {
      return false;  // COV_NF_LINE
    }
    cursor_ += size;
    return true;
  }

  // Writes the characters of the string to EEPROM; see WriteBytes for behavior
  // details. The caller is required to have some means of later determining the
  // size of the string that was written in order to read it, such as writing
  // the number of bytes in the string to EEPROM prior to writing the string's
  // value, or writing a fixed length.
  bool WriteString(const StringView& t) {
    return WriteBytes(reinterpret_cast<const uint8_t*>(t.data()), t.size());
  }

  // Reads `size` bytes from EEPROM into a char array. See ReadBytes for
  // behavior details.
  bool ReadString(char* ptr, const SizeT size) {
    return ReadBytes(reinterpret_cast<uint8_t*>(ptr), size);
  }

 protected:
  // First address in the region.
  const AddrT start_address_;
  // Size of the region.
  const SizeT size_;

  // Where we'll next read from or write to *within* this block (i.e. it is an
  // offset from start_), unless otherwise specified. Every successful read or
  // write operation sets this to offset of the byte immediately after that
  // of the byte that was last read from or written to.
  SizeT cursor_;
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_EEPROM_REGION_H_
