#ifndef MCUCORE_SRC_LOGGING_H_
#define MCUCORE_SRC_LOGGING_H_

// Logging and assert-like utility macros for Arduino sketches and similar
// environments. These are inspired by the Google Logging Library
// (https://github.com/google/glog), but are not identical. The features
// available are controlled by defining, or not, the macro MCU_ENABLE_CHECK and
// MCU_ENABLE_CHECK, and by the value of the macro MCU_ENABLED_VLOG_LEVEL
// (undefined or defined to an integer in the range 1 through 9).
//
// Author: james.synge@gmail.com
//
///////////////////////////////////////////////////////////////////////////////
//
// MCU_VLOG Usage:
//
// MCU_VLOG(level) << val1 << val2 << val3;
//
// If level is less than or equal to the value of MCU_ENABLED_VLOG_LEVEL,
// MCU_VLOG will print a line of text to the debug log sink (e.g. to Serial on
// Arduino); the text will start with the location of the statement (e.g.
// "file.cpp:123] "), followed by the string representations of val1, val2, and
// val3 concatenated together, and terminated by a newline.
//
// If the level is greater than the value of MCU_ENABLED_VLOG_LEVEL, or if
// MCU_ENABLED_VLOG_LEVEL is undefined, then no message is emitted, and if the
// compiler and linker are working as expected, the entire logging statement
// will be omitted from the compiled binary.
//
///////////////////////////////////////////////////////////////////////////////
//
// MCU_CHECK Usage:
//
// MCU_CHECK(expression) << message << values;
//
// The expression is always evaluated, but the result is only examined by the
// macro if MCU_ENABLE_CHECK is defined.
//
// In all cases, if expression is true, then no message is emitted and the
// statement completes normally (e.g. the next statement is executed or the
// scope is exited, as appropriate).
//
// When MCU_ENABLE_CHECK is defined and the expression is false, MCU_CHECK will
// emit a line of text indicating that the expression has failed, followed by
// the string representations of the message values inserted into MCU_CHECK, and
// finally a newline. On the host, the program will exit; on the Arduino, it
// will endlessly loop producing an error message; on the host it will exit the
// program.
//
// If MCU_ENABLE_CHECK is not defined and the expression is false, it will be
// treated as if true. This allows one to include code such as the following,
// and know that InitializeHardware will always be called when that statement is
// reached, regardless of whether MCU_ENABLE_CHECK is defined:
//
//     MCU_CHECK(InitializeHardware())
//         << MCU_FLASHSTR("Failed to initialize hardware.");
//
// This allows you to make extensive use of MCU_CHECK, yet know that the
// compiled size of the statement will shrink to that of the expression when
// MCU_ENABLE_CHECK is not defined.
//
// Note that we use the MCU_FLASHSTR macro here so that when compiling for AVR
// microcontrollers the string is stored only in Flash (PROGMEM), and is not
// copied to RAM.
//
// MCU_CHECK_NE, MCU_CHECK_EQ, etc. expand to a MCU_CHECK macro with the named
// comparison.
//
///////////////////////////////////////////////////////////////////////////////
//
// MCU_DCHECK Usage:
//
// MCU_DCHECK(expression) << message << values;
//
// Provides an assert like feature, similar to MCU_CHECK, but the expression is
// evaluated only if both MCU_ENABLE_CHECK and MCU_ENABLE_DCHECK are defined.
// Thus you can place many MCU_DCHECKs in your code for debugging, but then
// disable all of them while still leaving some critical MCU_CHECKs enabled.
//
// MCU_DCHECK_NE, MCU_DCHECK_EQ, etc. expand to a MCU_DCHECK macro with the
// named comparison.

#include "log_sink.h"
#include "mcucore_config.h"
#include "progmem_string_data.h"

// If explicitly requested, we disable certain features. This allows a single
// .cc file to disable logging. For example, request_decoder has lots of
// logging, but it is now well tested, so doesn't need much logging. Note that
// this wouldn't be necessary if logging.h supported per-module conditional
// logging; for embedded, this would be best done at compile time, which is a
// tad tricky.

#ifdef MCU_DISABLE_VLOG
#ifdef MCU_ENABLED_VLOG_LEVEL
#undef MCU_ENABLED_VLOG_LEVEL
#endif  // MCU_ENABLED_VLOG_LEVEL
#define MCU_ENABLED_VLOG_LEVEL 0
#endif  // MCU_DISABLE_VLOG

#ifdef MCU_DISABLE_CHECK
#ifdef MCU_ENABLE_CHECK
#undef MCU_ENABLE_CHECK
#endif  // MCU_ENABLE_CHECK
#endif  // MCU_DISABLE_CHECK

#ifdef MCU_DISABLE_DCHECK
#ifdef MCU_ENABLE_DCHECK
#undef MCU_ENABLE_DCHECK
#endif  // MCU_ENABLE_DCHECK
#endif  // MCU_DISABLE_DCHECK

#ifdef MCU_DISABLE_VLOG_LOCATION
#undef MCU_ENABLE_VLOG_LOCATION
#define MCU_VLOG_LOCATION(x) nullptr
#else
#define MCU_VLOG_LOCATION(x) MCU_BASENAME(x)
#endif

#ifdef MCU_DISABLE_CHECK_LOCATION
#undef MCU_ENABLE_CHECK_LOCATION
#define MCU_CHECK_LOCATION(x) nullptr
#else
#define MCU_CHECK_LOCATION(x) MCU_BASENAME(x)
#endif

#ifdef MCU_DISABLE_DCHECK_LOCATION
#undef MCU_ENABLE_DCHECK_LOCATION
#define MCU_DCHECK_LOCATION(x) nullptr
#else
#define MCU_DCHECK_LOCATION(x) MCU_BASENAME(x)
#endif

#define MCU_VOID_SINK ::mcucore::VoidSink()

#if defined(MCU_ENABLED_VLOG_LEVEL) && MCU_ENABLED_VLOG_LEVEL > 0

#define MCU_VLOG(level)                  \
  switch (0)                             \
  default:                               \
    (MCU_ENABLED_VLOG_LEVEL < level)     \
        ? (void)0                        \
        : ::mcucore::LogSinkVoidify() && \
              ::mcucore::LogSink(MCU_VLOG_LOCATION(__FILE__), __LINE__)

#define MCU_VLOG_IS_ON(level) (MCU_ENABLED_VLOG_LEVEL >= (level))

#else

#ifdef MCU_ENABLED_VLOG_LEVEL
#if MCU_ENABLED_VLOG_LEVEL < 0 || 9 < MCU_ENABLED_VLOG_LEVEL
#error "MCU_ENABLED_VLOG_LEVEL is out of range"
#endif
#endif  // MCU_ENABLED_VLOG_LEVEL

#define MCU_VLOG(level) \
  switch (0)            \
  default:              \
    (true) ? (void)0 : ::mcucore::LogSinkVoidify() && MCU_VOID_SINK

#define MCU_VLOG_IS_ON(level) (false)

#endif

// TODO(jamessynge): Consider adding MCU_ENABLE_NO_LOG_CHECK, which would log
// just the line number of a failed CHECK, rather than the full message, thus
// saving (a lot of?) PROGMEM used for storing message strings. This will make
// it harder to track down a failure, but not impossible.

#ifdef MCU_ENABLE_CHECK

#define MCU_CHECK_INTERNAL_(expression, message)                          \
  switch (0)                                                              \
  default:                                                                \
    (expression) ? (void)0                                                \
                 : ::mcucore::LogSinkVoidify() &&                         \
                       ::mcucore::CheckSink(MCU_CHECK_LOCATION(__FILE__), \
                                            __LINE__, MCU_FLASHSTR(message))

#else  // !MCU_ENABLE_CHECK

#define MCU_CHECK_INTERNAL_(expression, message) \
  switch (0)                                     \
  default:                                       \
    ((expression) || true) ? (void)0             \
                           : ::mcucore::LogSinkVoidify() && MCU_VOID_SINK

#endif  // MCU_ENABLE_CHECK

#define MCU_CHECK(expression) MCU_CHECK_INTERNAL_(expression, #expression)
#define MCU_CHECK_EQ(a, b) MCU_CHECK_INTERNAL_((a) == (b), #a " == " #b)
#define MCU_CHECK_NE(a, b) MCU_CHECK_INTERNAL_((a) != (b), #a " != " #b)
#define MCU_CHECK_LE(a, b) MCU_CHECK_INTERNAL_((a) <= (b), #a " <= " #b)
#define MCU_CHECK_LT(a, b) MCU_CHECK_INTERNAL_((a) < (b), #a " < " #b)
#define MCU_CHECK_GE(a, b) MCU_CHECK_INTERNAL_((a) >= (b), #a " >= " #b)
#define MCU_CHECK_GT(a, b) MCU_CHECK_INTERNAL_((a) > (b), #a " > " #b)

// If the MCU_CHECK* macros are enabled, then MCU_DCHECK* macros may also be
// enabled, but if MCU_CHECK* macros are disabled, then the MCU_DCHECK* macros
// are also disabled.

#if defined(MCU_ENABLE_CHECK) && defined(MCU_ENABLE_DCHECK)

#define MCU_DCHECK_INTERNAL_(expression, message)                           \
  switch (0)                                                                \
  default:                                                                  \
    (expression)                                                            \
        ? (void)0                                                           \
        : ::mcucore::LogSinkVoidify() &&                                    \
              ::mcucore::CheckSink(MCU_DCHECK_LOCATION(__FILE__), __LINE__, \
                                   MCU_FLASHSTR_128(message))

#else

#ifdef MCU_ENABLE_DCHECK
#undef MCU_ENABLE_DCHECK
#endif

#define MCU_DCHECK_INTERNAL_(expression, message) \
  switch (0)                                      \
  default:                                        \
    (true || (expression)) ? (void)0              \
                           : ::mcucore::LogSinkVoidify() && MCU_VOID_SINK

#endif

#define MCU_DCHECK(expression) MCU_DCHECK_INTERNAL_(expression, #expression)
#define MCU_DCHECK_EQ(a, b) MCU_DCHECK_INTERNAL_((a) == (b), #a " == " #b)
#define MCU_DCHECK_NE(a, b) MCU_DCHECK_INTERNAL_((a) != (b), #a " != " #b)
#define MCU_DCHECK_LE(a, b) MCU_DCHECK_INTERNAL_((a) <= (b), #a " <= " #b)
#define MCU_DCHECK_LT(a, b) MCU_DCHECK_INTERNAL_((a) < (b), #a " < " #b)
#define MCU_DCHECK_GE(a, b) MCU_DCHECK_INTERNAL_((a) >= (b), #a " >= " #b)
#define MCU_DCHECK_GT(a, b) MCU_DCHECK_INTERNAL_((a) > (b), #a " > " #b)

#endif  // MCUCORE_SRC_LOGGING_H_
