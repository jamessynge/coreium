#ifndef MCUCORE_SRC_INT_HELPERS_H_
#define MCUCORE_SRC_INT_HELPERS_H_

// Provides functions for converting integers.

#include "mcucore_platform.h"
#include "type_traits.h"

namespace mcucore {

// ToUnsigned(T value) returns the integer value as an unsigned integer of a
// type that is the same size as T.

template <typename T,
          enable_if_t<sizeof(T) == sizeof(uint8_t) && is_integral<T>::value &&
                          !is_same<T, bool>::value,
                      int> = 1>
uint8_t ToUnsigned(const T value) {
  return static_cast<uint8_t>(value);
}

template <typename T,
          enable_if_t<sizeof(T) == sizeof(uint16_t) && is_integral<T>::value,
                      int> = 2>
uint16_t ToUnsigned(const T value) {
  return static_cast<uint16_t>(value);
}

template <typename T,
          enable_if_t<sizeof(T) == sizeof(uint32_t) && is_integral<T>::value,
                      int> = 3>
uint32_t ToUnsigned(const T value) {
  return static_cast<uint32_t>(value);
}

template <typename T,
          enable_if_t<sizeof(T) == sizeof(uint64_t) && is_integral<T>::value,
                      int> = 4>
uint64_t ToUnsigned(const T value) {
  return static_cast<uint64_t>(value);
}

}  // namespace mcucore

#endif  // MCUCORE_SRC_INT_HELPERS_H_
