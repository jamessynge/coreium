#ifndef MCUCORE_SRC_PROGMEM_STRING_VIEW_H_
#define MCUCORE_SRC_PROGMEM_STRING_VIEW_H_

// ProgmemStringView is like StringView, but for a view of a string stored in
// program memory (aka PROGMEM), rather than in read-write memory (aka RAM). On
// microcontrollers, this typically means flash memory.
//
// But why do we need a separate type for a view on a read-only string? Because
// AVR microcontrollers have multiple address spaces, meaning that we can't tell
// from the value of an address whether it is in RAM or Flash, or EEPROM for
// that matter; we need to know also which space it is in. This multiple address
// spaces design is known as Harvard Architecture.
//
// So, ProgmemStringView has support for using alternate instructions (via AVR
// Libc's progmem library) to access the characters in a string.
//
// NOTE: So far I've written this using PGM_P pointers, which, IIUC, are limited
// to the first 64KB of flash. I don't know what guarantees there are about the
// placement of variables marked PROGMEM, in particular whether there is an
// attempt to place them early in the address space.
//
// Author: james.synge@gmail.com

#include "has_progmem_char_array.h"
#include "logging.h"
#include "mcucore_platform.h"
#include "progmem_pointer.h"
#include "progmem_string_data.h"
#include "type_traits.h"

namespace mcucore {

class ProgmemStringView {
 public:
  // These two definitions must be changed together.
  using size_type = uint8_t;
  static constexpr size_type kMaxSize = 255;

  // Construct empty.
  constexpr ProgmemStringView() noexcept : ptr_(nullptr), size_(0) {}

  constexpr ProgmemStringView(PGM_P ptr, size_type length)
      : ptr_(ptr), size_(length) {}

  // Constructs from a string literal stored in AVR PROGMEM (or regular memory
  // on other CPU types).
  //
  // NOTE: The length of a C++ string literal includes the NUL (\0) at the end,
  // so we subtract one from N to get the length of the string before that, and
  // that value, M, must be be representable with ProgmemStringView::size_type.
  template <size_t N, size_type M = N - 1>
  explicit constexpr ProgmemStringView(const char (&buf)[N] PROGMEM)
      : ptr_(buf), size_(M) {}

  // Constructs from the value returned by the `progmem_char_array` member
  // function of the arg.
  template <typename PSD,
            typename = enable_if_t<has_progmem_char_array<PSD>::value>>
  constexpr ProgmemStringView(PSD p)  // NOLINT: Want this to be implicit.
      : ProgmemStringView(p.progmem_char_array()) {}

  // Copy constructor and assignment operator.
  constexpr ProgmemStringView(const ProgmemStringView&) = default;
  ProgmemStringView& operator=(const ProgmemStringView&) = default;

  // Print the string to the provided Print instance.
  size_t printTo(Print& out) const;

  // Returns true if the other literal has the same value.
  bool operator==(const ProgmemStringView& other) const;

  // Returns true if the other literal has a different value.
  bool operator!=(const ProgmemStringView& other) const {
    return !(*this == other);
  }

  // Returns true if the two instances are identical views of the same PROGMEM
  // string. This is really useful just for testing that the compiler and linker
  // are combining strings together as expected.
  bool Identical(const ProgmemStringView& other) const;

  // Returns true if the two strings are equal, with case sensitive comparison
  // of characters. other points to a string in RAM, not PROGMEM.
  bool Equal(const char* other, size_type other_size) const;

  // Returns true if the two strings are equal, with case insensitive comparison
  // of characters. other points to a string in RAM, not PROGMEM.
  bool CaseEqual(const char* other, size_type other_size) const;

  // Returns true if the two strings are equal, after lower-casing this PROGMEM
  // string. other points to a string in RAM, not PROGMEM.
  bool LoweredEqual(const char* other, size_type other_size) const;

  // Returns true if the other string starts with this literal string.
  bool IsPrefixOf(const char* other, size_type other_size) const;

  // If 'size_' is not greater than the provided 'size', copies the literal
  // string into *out. No NUL terminator is copied.
  bool CopyTo(char* out, size_type size);

  // Support for iterating.
  constexpr ProgMemCharPtr begin() const { return ProgMemCharPtr(ptr_); }
  constexpr ProgMemCharPtr end() const { return ProgMemCharPtr(ptr_ + size_); }

  // Returns the character the the specified position ([0..size_)) within the
  // string.
  char at(size_type pos) const;

  // Returns the number of characters in the string.
  constexpr size_type size() const { return size_; }

  // Returns a pointer to the underlying data, in some address space.
  constexpr PGM_VOID_P progmem_ptr() const {
    return reinterpret_cast<PGM_VOID_P>(ptr_);
  }

  // Returns a view of a portion of this view (at offset `pos` and length `n`)
  // as another StringView. Does NOT validate the parameters, so pos+n must not
  // be greater than size(). This is currently only used for non-embedded
  // code, hence the DCHECKs instead of ensuring that the result is valid.
  ProgmemStringView substr(size_type pos, size_type n) const {
    MCU_DCHECK_LE(pos, size_);
    MCU_DCHECK_LE(pos + n, size_);
    return ProgmemStringView(ptr_ + pos, n);
  }

 private:
  PGM_P ptr_;
  size_type size_;
};

template <class PSD, typename = enable_if_t<has_progmem_char_array<PSD>::value>>
constexpr ProgmemStringView MakeProgmemStringView() {
  return ProgmemStringView(PSD::kData, (sizeof PSD::kData) - 1);
}

}  // namespace mcucore

////////////////////////////////////////////////////////////////////////////////
// We define below macros MCU_PSV_nnn (PSV==ProgmemStringView) for various
// values of nnn, which represents the maximum length of string literal (not
// including the terminating null character) supported by the macro. These
// produce *values* of type ProgmemStringView that can be printed or otherwise
// operated upon at runtime.

#define MCU_PSV_32(x) \
  (::mcucore::MakeProgmemStringView<decltype(MCU_PSD_32(x))>())

#define MCU_PSV_64(x) \
  (::mcucore::MakeProgmemStringView<decltype(MCU_PSD_64(x))>())

#define MCU_PSV_128(x) \
  (::mcucore::MakeProgmemStringView<decltype(MCU_PSD_128(x))>())

// Max length 255 (not including trailing NUL). This is not a power of two
// because ProgmemStringView uses a uint8 to record the size of the string, and
// can't represent 256.

#define MCU_PSV_255(x) \
  (::mcucore::MakeProgmemStringView<decltype(MCU_PSD_255(x))>())

#define MCU_PSV(x) MCU_PSV_64(x)

#endif  // MCUCORE_SRC_PROGMEM_STRING_VIEW_H_
