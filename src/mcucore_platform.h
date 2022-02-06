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

using MillisT = unsigned long;  // NOLINT
using MicrosT = unsigned long;  // NOLINT

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

using MillisT = uint32_t;
using MicrosT = uint32_t;

#endif  // ARDUINO

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

// max is a macro in Arduino, but not on a host. We avoid 'confusion' by using
// inlineable function.
constexpr inline size_t MaxOf2(const size_t a, const size_t b) {
#ifdef ARDUINO
  return max(a, b);
#else   // !ARDUINO
  return (a >= b) ? a : b;
#endif  // ARDUINO
}
constexpr size_t MaxOf4(size_t a, size_t b, size_t c, size_t d) {
  return MaxOf2(MaxOf2(a, b), MaxOf2(c, d));
}

// It turns out that absl/meta/type_traits.h uses the symbol F in a template
// definition, and Arduino's WString.h definition of macro F(s) interferes if
// the former is included after the latter. Avoiding this problem by using
// FLASHSTR in place of F on the host.
#ifdef ARDUINO
#define FLASHSTR(string_literal) F(string_literal)
#else  // !ARDUINO
#define FLASHSTR(string_literal) \
  (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))
#endif  // ARDUINO

// Test for the has_feature intrinsic offered by some compilers.
#ifdef __has_feature
#define MCU_HAS_FEATURE(x) __has_feature(x)
#elif defined(__has_extension)
#define MCU_HAS_FEATURE(x) __has_extension(x)
#else
#define MCU_HAS_FEATURE(x) 0
#endif

#endif  // MCUCORE_SRC_MCUCORE_PLATFORM_H_
