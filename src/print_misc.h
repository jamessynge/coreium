#ifndef MCUCORE_SRC_PRINT_MISC_H_
#define MCUCORE_SRC_PRINT_MISC_H_

// Miscellaneous print functions, in a separate compilation unit to avoid
// dependency cycles.
//
// Author: james.synge@gmail.com

#include "has_progmem_char_array.h"
#include "mcucore_platform.h"

namespace mcucore {

// Utility function supporting the printing of enumerator names, in this case
// handling an enum value that is not a defined enumerator.
size_t PrintUnknownEnumValueTo(const __FlashStringHelper* name, uint32_t v,
                               Print& out);

// Prints the specified flash string, whose length is known, to out. This
// provides an optimization for AVR chips by copying sections of the string into
// RAM, then printing those.
size_t PrintFlashStringOfLength(const __FlashStringHelper* ptr, size_t length,
                                Print& out);

// Print the specified ProgmemStringData to out.
template <typename PSD,
          typename = enable_if_t<has_progmem_char_array<PSD>::value>>
inline size_t PrintProgmemStringData(const PSD str, Print& out) {
  return PrintFlashStringOfLength(
      reinterpret_cast<const __FlashStringHelper*>(str.progmem_char_array()),
      str.size(), out);
}

}  // namespace mcucore

#endif  // MCUCORE_SRC_PRINT_MISC_H_
