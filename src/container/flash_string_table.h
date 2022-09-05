#ifndef MCUCORE_SRC_CONTAINER_FLASH_STRING_TABLE_H_
#define MCUCORE_SRC_CONTAINER_FLASH_STRING_TABLE_H_

// Support for storing in flash tables of pointers to strings that are
// themselves stored in flash; i.e. a PROGMEM array of PROGMEM strings.
//
// NOTE: So far this only works if the table is stored in the first 64KB of
// flash because this uses the "near pointer" API.
//
// TODO(jamessynge): Consider whether to add a form of the table as a map from
// sparse enum values to flash strings. Could use template metaprogramming to
// sort entries by enum value, and maybe use the same for eliminating multiple
// enumerators with the same value.
//
// TODO(jamessynge): Add examples showing how to use this.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "strings/has_progmem_char_array.h"

namespace mcucore {
namespace flash_string_table_internal {

// Holds a pointer to a string stored in flash, where the instance is itself
// stored in flash, so we must call ToFlashStringHelper to read the pointer from
// flash.
class FlashStringTableElement {
 public:
  // constexpr FlashStringTableElement(const __FlashStringHelper* ptr)  //
  // NOLINT
  //     : ptr_(ptr) {}

  template <typename PSD,
            typename = enable_if_t<has_progmem_char_array<PSD>::value>>
  constexpr FlashStringTableElement(const PSD /*value*/)  // NOLINT
      : ptr_(PSD::kData) {}

  const __FlashStringHelper* ToFlashStringHelper() const;

 private:
  const char* ptr_;
};

}  // namespace flash_string_table_internal

using FlashStringTable =
    const flash_string_table_internal::FlashStringTableElement[];

// Returns the value of the `entry`-th element in the table, which has size
// `table_size`.
template <typename T>
const __FlashStringHelper* LookupFlashString(FlashStringTable table,
                                             T table_size, T entry) {
  if (entry < table_size) {
    return table[entry].ToFlashStringHelper();
  } else {
    return nullptr;
  }
}

// Returns the value of the `entry`-th element in the table.
template <typename T, T N>
const __FlashStringHelper* LookupFlashString(
    const flash_string_table_internal::FlashStringTableElement (&table)[N],
    T entry) {
  return LookupFlashString(table, N, entry);
}

// Lookup for the case where min_enumerator might be non-zero.
// TODO(jamessynge): Add static_assert or similar compile time check that the
// min and max enumerators are appropriate for the size of the table.
template <typename U, typename E>
const __FlashStringHelper* LookupFlashStringForDenseEnum(FlashStringTable table,
                                                         E min_enumerator,
                                                         E max_enumerator,
                                                         E enumerator_value) {
  if (min_enumerator <= enumerator_value &&
      enumerator_value <= max_enumerator) {
    return table[static_cast<U>(enumerator_value) -
                 static_cast<U>(min_enumerator)]
        .ToFlashStringHelper();
  } else {
    return nullptr;
  }
}

template <typename U, typename E, U N>
const __FlashStringHelper* LookupFlashStringForDenseEnum(
    const flash_string_table_internal::FlashStringTableElement (&table)[N],
    E enumerator_value) {
  return LookupFlashString(table, N, static_cast<U>(enumerator_value));
}

}  // namespace mcucore

#define MCU_FLASH_STRING_TABLE(table_name, strings...)                  \
  const ::mcucore::flash_string_table_internal::FlashStringTableElement \
      table_name[] AVR_PROGMEM = {strings}

#endif  // MCUCORE_SRC_CONTAINER_FLASH_STRING_TABLE_H_
