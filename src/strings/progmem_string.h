#ifndef MCUCORE_SRC_STRINGS_PROGMEM_STRING_H_
#define MCUCORE_SRC_STRINGS_PROGMEM_STRING_H_

// ProgmemString encapsulates a pointer to a string literal; i.e. a pointer to a
// NUL-terminated string stored in program memory. This is equivalent to using
// Arduino's 'const __FlashStringHelper*', but makes it somewhat easier to
// provide operations on the values.
//
// Avoiding the use of 'const char*' as the type of a string literal helps deal
// with processor architectures with multiple address spaces, e.g. the Microchip
// AVR series, where address X alone isn't sufficient to identify whether X
// refers to RAM, Program Memory (e.g. flash), or some other space.
//
// NOTE: So far I've written this using char* pointers, which, IIUC, are limited
// to the first 64KB of flash on AVR chips. I don't know what guarantees there
// are about the placement of variables marked PROGMEM, in particular whether
// there is an attempt to place them early in the address space.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "semistd/type_traits.h"
#include "strings/has_progmem_char_array.h"

namespace mcucore {

class ProgmemString {
 public:
  // Constructs from a value produced by Arduino's F(str) macro, or McuCore's
  // FLASHSTR(str), which is a direct equivalent of 'F(str)', or from
  // 'MCU_FLASHSTR(str)', which has the benefit of collapsing multiple copies of
  // the same string into a single string in flash.
  ProgmemString(const __FlashStringHelper* str)  // NOLINT
      : ptr_(reinterpret_cast<const char*>(str)) {}

  // Constructs from the value returned by the `progmem_char_array` member
  // function of the arg.
  template <typename PSD,
            typename = enable_if_t<has_progmem_char_array<PSD>::value>>
  constexpr ProgmemString(const PSD p)  // NOLINT: Should be implicit
      : ptr_(p.progmem_char_array()) {}

  ProgmemString() : ptr_(nullptr) {}

  constexpr ProgmemString(const ProgmemString&) = default;

  // Returns true if other has the same value.
  bool operator==(const ProgmemString& other) const;

  // Returns true if other has a different value.
  bool operator!=(const ProgmemString& other) const {
    return !(*this == other);
  }

  // Returns true if empty (i.e. if there is no progmem string).
  bool empty() const { return ptr_ == nullptr; }

  // Return a __FlashStringHelper*, the type used by Arduino's Print class to
  // help avoid copying strings into RAM.
  const __FlashStringHelper* ToFlashStringHelper() const {
    return reinterpret_cast<const __FlashStringHelper*>(ptr_);
  }

  // Print the string to the provided Print instance. This is not a virtual
  // function (i.e. not an override of Printable::printTo) because that would
  // remove the ability for this to have any constexpr ctor in C++ < 20.
  inline size_t printTo(Print& out) const {
    return out.print(ToFlashStringHelper());
  }

 private:
  // Pointer to a string in program memory (i.e. flash), rather than in RAM.
  const char* ptr_;
};

// This is an array of ProgmemString instances, stored in RAM, not PROGMEM.
//
// NOTE: due to C++ 11 limitations (where 11 is the version supported by Arduino
// as of 2021), we can't (easily?) construct a constexpr initializer list, in
// particular one stored in PROGMEM, to support these kinds of initialization:
//
//    constexpr ProgmemStringArray a = ProgmemStringArray{str2, str1};
//    constexpr ProgmemStringArray b = {str2, str1};
struct ProgmemStringArray {
  constexpr ProgmemStringArray() : array(nullptr), size(0) {}

  template <size_t N>
  explicit constexpr ProgmemStringArray(
      const ProgmemString (&progmem_strings)[N])
      : array(progmem_strings), size(N) {}

  constexpr ProgmemStringArray(const ProgmemStringArray&) = default;

  const ProgmemString* begin() const { return array; }
  const ProgmemString* end() const { return array + size; }

  const ProgmemString* array;
  const size_t size;
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_STRINGS_PROGMEM_STRING_H_
