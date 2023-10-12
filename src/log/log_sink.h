#ifndef MCUCORE_SRC_LOG_LOG_SINK_H_
#define MCUCORE_SRC_LOG_LOG_SINK_H_

// LogSink is used for printing a message to a Print instance.
//
// CheckSink is used for printing a fatal failure message to a Print instance.
//
// VoidSink is used in place of LogSink when a logging statement is disabled at
// compile time (e.g. if the level passed to MCU_VLOG is too high, or if
// MCU_DCHECK is disabled).
//
// DEFAULT_SINK_OUT is Serial when compiled for the Arduino; for the host, it is
// stderr, though that may be overridden by SetPrintForLogSink and
// SetPrintForCheckSink, as appropriate.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"

#if MCU_HOST_TARGET
#include <functional>   // pragma: keep standard include
#include <string_view>  // pragma: keep standard include
#endif

#include "print/o_print_stream.h"
#include "strings/progmem_string.h"

namespace mcucore {

class LogSink final : public OPrintStream {
 public:
  // Streams to DEFAULT_SINK_OUT, with a prefix determined by file and
  // line_number. The prefix has the format "file.ext:line_number] " if file_
  // points to a non-empty string; omits ":line_number" if line_number_ is
  //   zero. The prefix is empty if file is null or empty.
  LogSink(ProgmemString file, uint16_t line_number);

  // Streams to DEFAULT_SINK_OUT, omits the log location prefix.
  LogSink();

  // Writes a newline and flushes the output.
  ~LogSink();
};

class CheckSink : public OPrintStream {
 public:
  // Streams to  "MCU_CHECK FAILED: file.ext:line_number] expression_message "
  CheckSink(ProgmemString file, uint16_t line_number,
            ProgmemString expression_message);

  // TODO(jamessynge): Mark the dtor as [[noreturn]], if it helps with writing
  // non-void functions that end with an MCU_CHECK (i.e. so we don't have to add
  // a return statement after that). If that attribute works as desired, we'll
  // have to change the tests which make use of SetCheckSinkExitFn, such as by
  // eliminating that function and/or by adding a separate DebugCheckSink class,
  // whose dtor is not marked with [[noreturn]], and a SetDebugCheckSinkExitFn.
  ~CheckSink();
};

// A stream sink to be used in place of LogSink or CheckSink when they've been
// disabled at compiled time (e.g. by the level passed to MCU_VLOG being higher
// than MCU_ENABLED_VLOG_LEVEL).
class VoidSink {
 public:
  VoidSink() {}
  ~VoidSink() {}

  template <typename T>
  VoidSink& operator<<(const T&) {
    return *this;
  }
};

// LogSinkVoidify is used to produce a void value in MCU_VLOG, etc. It is based
// on https://github.com/google/asylo/blob/master/asylo/util/logging.h
//
// In MCU_VLOG, MCU_VLOG_IF, etc., we use the conditional operator (aka the
// ternary operator), yielding an expression such as:
//
//   do-not-log-bool-expression ? (void)0 : LogSink << logged << values
//
// However, that won't work as is because the type of the second and third
// operands must be the same. Since we want void as the type of the whole
// expression, thus preventing it from being treated as a value, we need some
// way to make the type of the third operand be void. We do this by applying
// an operator to it whose return type is void. In particular, we make our third
// operand look like this:
//
//     LogSinkVoidify() && LogSink(args) << logged << values
//
// While LogSinkVoidify and LogSink aren't void, we define operator&& for
// LogSinkVoidify on the LHS and LogSink (really OPrintStream) on the RHS, with
// the return type of the operator being void. We use && rather than some other
// operator because it has lower precedence than <<, and thus the third operand
// is parsed as:
//
//     LogSinkVoidify() && (LogSink(args) << logged << values)
//
// i.e. the logged values are passed into the operator<< method defined by
// OPrintStream. VoidLogSink is used  we use && as the operator,
class LogSinkVoidify {
 public:
  // We use operator&& here because it has precedence lower than `<<` but higher
  // than the ternary operator `:?`.
  void operator&&(const OPrintStream&) {}
  void operator&&(const VoidSink&) {}
};

#if MCU_HOST_TARGET
// To enable testing of the log sinks and logging macros, we allow the default
// printers to be replaced, along with the function called when an MCU_CHECK
// statement is done streaming values.
void SetPrintForLogSink(Print* out);
void SetPrintForCheckSink(Print* out);
using CheckSinkExitFn = std::function<void(std::string_view)>;
void SetCheckSinkExitFn(CheckSinkExitFn exit_fn);
#endif

}  // namespace mcucore

#endif  // MCUCORE_SRC_LOG_LOG_SINK_H_
