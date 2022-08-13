#ifndef MCUCORE_SRC_SEMISTD_LIMITS_H_
#define MCUCORE_SRC_SEMISTD_LIMITS_H_

// A very minimal version of std::numeric_limits.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "semistd/type_traits.h"

namespace mcucore {

namespace numeric_limits_internal {

template <typename T, bool IsSigned, int Size>
struct numeric_limits {};

template <typename T>
struct numeric_limits<T, true, 1> {
  static constexpr T min() {
    static_assert(static_cast<T>(INT8_MIN) == INT8_MIN);
    return static_cast<T>(INT8_MIN);
  }
  static constexpr T max() { return static_cast<T>(INT8_MAX); }
  static constexpr bool is_signed = true;  // NOLINT
  static constexpr bool is_exact = true;   // NOLINT
};

template <typename T>
struct numeric_limits<T, false, 1> {
  static constexpr T min() { return 0; }
  static constexpr T max() {
    static_assert(static_cast<T>(UINT8_MAX) == UINT8_MAX);
    return static_cast<T>(UINT8_MAX);
  }
  static constexpr bool is_signed = false;  // NOLINT
  static constexpr bool is_exact = true;    // NOLINT
};

template <typename T>
struct numeric_limits<T, true, 2> {
  static constexpr T min() {
    static_assert(static_cast<T>(INT16_MIN) == INT16_MIN);
    return static_cast<T>(INT16_MIN);
  }
  static constexpr T max() { return static_cast<T>(INT16_MAX); }
  static constexpr bool is_signed = true;  // NOLINT
  static constexpr bool is_exact = true;   // NOLINT
};

template <typename T>
struct numeric_limits<T, false, 2> {
  static constexpr T min() { return 0; }
  static constexpr T max() {
    static_assert(static_cast<T>(UINT16_MAX) == UINT16_MAX);
    return static_cast<T>(UINT16_MAX);
  }
  static constexpr bool is_signed = false;  // NOLINT
  static constexpr bool is_exact = true;    // NOLINT
};

template <typename T>
struct numeric_limits<T, true, 4> {
  static constexpr T min() {
    static_assert(static_cast<T>(INT32_MIN) == INT32_MIN);
    return static_cast<T>(INT32_MIN);
  }
  static constexpr T max() { return static_cast<T>(INT32_MAX); }
  static constexpr bool is_signed = true;  // NOLINT
  static constexpr bool is_exact = true;   // NOLINT
};

template <typename T>
struct numeric_limits<T, false, 4> {
  static constexpr T min() { return 0; }
  static constexpr T max() {
    static_assert(static_cast<T>(UINT32_MAX) == UINT32_MAX);
    return static_cast<T>(UINT32_MAX);
  }
  static constexpr bool is_signed = false;  // NOLINT
  static constexpr bool is_exact = true;    // NOLINT
};

template <typename T>
struct numeric_limits<T, true, 8> {
  static constexpr T min() {
    static_assert(static_cast<T>(INT64_MIN) == INT64_MIN);
    return static_cast<T>(INT64_MIN);
  }
  static constexpr T max() { return static_cast<T>(INT64_MAX); }
  static constexpr bool is_signed = true;  // NOLINT
  static constexpr bool is_exact = true;   // NOLINT
};

template <typename T>
struct numeric_limits<T, false, 8> {
  static constexpr T min() { return 0; }
  static constexpr T max() {
    static_assert(static_cast<T>(UINT64_MAX) == UINT64_MAX);
    return static_cast<T>(UINT64_MAX);
  }
  static constexpr bool is_signed = false;  // NOLINT
  static constexpr bool is_exact = true;    // NOLINT
};

}  // namespace numeric_limits_internal

template <typename T>
struct numeric_limits
    : numeric_limits_internal::numeric_limits<T, is_signed<T>::value,
                                              sizeof(T)> {};

}  // namespace mcucore

#endif  // MCUCORE_SRC_SEMISTD_LIMITS_H_
