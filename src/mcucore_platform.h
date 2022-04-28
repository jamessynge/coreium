#ifndef MCUCORE_SRC_MCUCORE_PLATFORM_H_
#define MCUCORE_SRC_MCUCORE_PLATFORM_H_

// Provides platform setup and exporting of platform specific header files.
//
// Why export all of these includes? Because otherwise lots of files have lots
// of includes with conditional compliation guards around them, which is pretty
// messy.
//
// Author: james.synge@gmail.com

#include "mcucore_config.h"

#ifdef ARDUINO

#define MCU_EMBEDDED_TARGET 1
#define MCU_HOST_TARGET 0
#define MCU_ENABLE_DEBUGGING 0

#include <Arduino.h>  // IWYU pragma: export
#include <EEPROM.h>

#ifdef ARDUINO_ARCH_AVR
#include <avr/pgmspace.h>
#define AVR_PROGMEM PROGMEM
#else
#define AVR_PROGMEM
#endif  // ARDUINO_ARCH_AVR

#else  // !ARDUINO

#define MCU_EMBEDDED_TARGET 0
#define MCU_HOST_TARGET 1

#ifdef NDEBUG
#define MCU_ENABLE_DEBUGGING 0
#else
#define MCU_ENABLE_DEBUGGING 1
#endif  // NDEBUG

#include "absl/base/attributes.h"
#include "extras/host/arduino/arduino.h"  // IWYU pragma: export  // pragma: keep extras include
#include "extras/host/arduino/pgmspace.h"  // IWYU pragma: export  // pragma: keep extras include
#include "extras/host/arduino/print.h"  // IWYU pragma: export  // pragma: keep extras include
#include "extras/host/arduino/stream.h"  // IWYU pragma: export  // pragma: keep extras include
#include "extras/host/eeprom/eeprom.h"  // IWYU pragma: export  // pragma: keep extras include

#define AVR_PROGMEM

#endif  // ARDUINO

// The names of some Arduino macros are the same as those of symbols found in
// useful libraries, interfering with their use or with my own definition of
// symbols with those names (e.g. <limits> defines functions called min and
// max). Where I find that to be the case, I undefine those macros here and work
// to find an alternative in McuCore, et al.

// #undef abs
#undef max       // <limits>
#undef min       // <limits>
#undef round     // <chrono>
#undef INTERNAL  // codes.proto.h
#undef EXTERNAL  // absl/strings/cord.h (or included files).
#undef F         // absl/meta/type_traits.h; see FLASHSTR below and MCU_FLASHSTR
                 // in progmem_string_data.h.
#undef DEFAULT

template <typename T>
T max(T a, T b) {
  return a >= b ? a : b;
}
template <typename T>
T min(T a, T b) {
  return a <= b ? a : b;
}

namespace mcucore {
// A common definition of the type to be used for indexing into EEPROM.
using EepromAddrT = decltype(EEPROM.length());

// The types in which milliseconds and microseconds are expressed.
using MillisT = decltype(millis());
using MicrosT = decltype(micros());

// Returns milliseconds since start_time.
MillisT ElapsedMillis(MillisT start_time);
}  // namespace mcucore

// See absl/base/attributes.h.
#ifdef ABSL_FALLTHROUGH_INTENDED
#define MCU_FALLTHROUGH_INTENDED ABSL_FALLTHROUGH_INTENDED
#elif defined(__GNUC__) && __GNUC__ >= 7
#define MCU_FALLTHROUGH_INTENDED [[gnu::fallthrough]]
#else
#define MCU_FALLTHROUGH_INTENDED \
  do {                           \
  } while (0)
#endif

// If a function contains a MCU_VLOG, et al (e.g. when compiled for debugging),
// then the function can't be a constexpr. To allow for including these macros
// in such functions, we use these macros to choose whether it is a constexpr or
// not based on whether we've compiled it for debugging or not.
#if MCU_ENABLE_DEBUGGING
#define MCU_CONSTEXPR_FUNC
#define MCU_CONSTEXPR_VAR const
#else
#define MCU_CONSTEXPR_FUNC constexpr
#define MCU_CONSTEXPR_VAR constexpr
#endif  // MCU_ENABLE_DEBUGGING

// It turns out that absl/meta/type_traits.h uses the symbol F in a template
// definition, and Arduino's WString.h definition of macro F(s) interferes if
// the former is included after the latter. Avoiding this problem by using
// FLASHSTR in place of F. Note that this is AVR specific.
#define FLASHSTR(string_literal) \
  (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

// Test for the has_feature intrinsic offered by some compilers.
#ifdef __has_feature
#define MCU_HAS_FEATURE(x) __has_feature(x)
#elif defined(__has_extension)
#define MCU_HAS_FEATURE(x) __has_extension(x)
#else
#define MCU_HAS_FEATURE(x) 0
#endif

#ifndef __cplusplus
#define __cplusplus 0
#endif

// Concatenates macro args, which is necessary when attempting to concatenate
// args and pass them to another macro due to the order that the preprocessor
// evaluates tokens in a macro expansion.
#define MCU_CONCAT_NAME(x, y) MCU_CONCAT_NAME_INNER_(x, y)
#define MCU_CONCAT_NAME_INNER_(x, y) x##y

// MCU_UNIQUE_NAME helps with generating a unique symbol name. If __COUNTER__,
// a non-standard feature is available, we'll use it, else we'll fallback to
// using __LINE__ has the unique-ifying value; that works as long as we don't
// need to generate more than one such name on a single line.
#if defined(__clang__) || defined(__GNUC__)
#define MAKE_UNIQUE_NAME(base_name) MCU_CONCAT_NAME(base_name, __COUNTER__)
#else
#define MAKE_UNIQUE_NAME(base_name) MCU_CONCAT_NAME(base_name, __LINE__)
#endif

// For now, C++ 14 features are not enabled.
#ifdef MCU_HAS_CXX14_FEATURES

// Adapted from boost's gcc_xml.hpp, providing a straight forward check for
// whether the specified feature is available.
#if defined(__cpp_return_type_deduction) && \
    (__cpp_return_type_deduction >= 201304)
#define MCU_CXX14_RETURN_TYPE_DEDUCTION
#endif
#if defined(__cpp_variable_templates) && (__cpp_variable_templates >= 201304)
#define MCU_CXX14_VARIABLE_TEMPLATES
#endif

#endif  // MCU_HAS_CXX14_FEATURES

#endif  // MCUCORE_SRC_MCUCORE_PLATFORM_H_
