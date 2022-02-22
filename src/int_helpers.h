#ifndef MCUCORE_SRC_INT_HELPERS_H_
#define MCUCORE_SRC_INT_HELPERS_H_

// Provides functions for converting integers.

#include "mcucore_platform.h"
#include "type_traits.h"

namespace mcucore {

template <typename T, enable_if_t<sizeof(T) == sizeof(uint8_t), int> = 1>
uint8_t ToUnsigned(T value) {
  return static_cast<uint8_t>(value);
}

template <typename T, enable_if_t<sizeof(T) == sizeof(uint16_t), int> = 2>
uint16_t ToUnsigned(T value) {
  return static_cast<uint16_t>(value);
}

template <typename T, enable_if_t<sizeof(T) == sizeof(uint32_t), int> = 3>
uint32_t ToUnsigned(T value) {
  return static_cast<uint32_t>(value);
}

template <typename T, enable_if_t<sizeof(T) == sizeof(uint64_t), int> = 4>
uint64_t ToUnsigned(T value) {
  return static_cast<uint64_t>(value);
}

}  // namespace mcucore

#endif  // MCUCORE_SRC_INT_HELPERS_H_
