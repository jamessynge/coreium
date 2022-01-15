#ifndef MCUCORE_SRC_LOG_SINK_H_
#define MCUCORE_SRC_LOG_SINK_H_

// LogSink is used for printing a message to a Print instance.
//
// CheckSink is used for printing a fatal failure message to a Print instance.
//
// VoidSink is used in place of LogSink when a logging statement is disabled at
// compile time (e.g. if the level passed to MCU_VLOG is too high, or if
// MCU_DCHECK is disabled).
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"

#if MCU_HOST_TARGET
#include <functional>   // pragma: keep standard include
#include <string_view>  // pragma: keep standard include
#endif

#include "o_print_stream.h"

namespace mcucore {

class MessageSinkBase : public OPrintStream {
 public:
  MessageSinkBase(Print& out, const __FlashStringHelper* file,
                  uint16_t line_number);

 protected:
  // Prints the location (e.g. "filename.ext:line_number] ") to out, if file_ is
  // points to a non-empty string. Omits ":line_number" if line_number_ is zero.
  void PrintLocation(Print& out) const;

 private:
  const __FlashStringHelper* const file_;
  const uint16_t line_number_;
};

class LogSink final : public MessageSinkBase {
 public:
  // Streams to out, with a prefix of the file and line_number, as described by
  // PrintLocation above.
  LogSink(Print& out, const __FlashStringHelper* file, uint16_t line_number);
  // As above, but with the Print instance set to DEFAULT_SINK_OUT, which is
  // Serial on Arduino, and FakeSerial on host (i.e. stdout).
  LogSink(const __FlashStringHelper* file, uint16_t line_number);
  // Omits the log location, i.e. file defaults to nullptr.
  explicit LogSink(Print& out);
  // Streams to DEFAULT_SINK_OUT, omits the log location.
  LogSink();

  // Writes a newline and flushes the output.
  ~LogSink();
};

class CheckSink : public MessageSinkBase {
 public:
  CheckSink(const __FlashStringHelper* file, uint16_t line_number,
            const __FlashStringHelper* expression_message);
  ~CheckSink();

 private:
  void Announce(Print& out) const;

  const __FlashStringHelper* const expression_message_;
};

class VoidSink {
 public:
  VoidSink() {}
  ~VoidSink() {}

  template <typename T>
  VoidSink& operator<<(const T&) {
    return *this;
  }
};

// Based on https://github.com/google/asylo/blob/master/asylo/util/logging.h
// This class is used just to take a type used as a log sink (i.e. the LHS of
// insert operators in log statements) and make it a void type to satisify the
// ternary operator in MCU_VLOG, MCU_CHECK and MCU_DCHECK. `operand&&` is used
// because it has precedence lower than `<<` but higher than the ternary
// operator `:?`

class LogSinkVoidify {
 public:
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

#endif  // MCUCORE_SRC_LOG_SINK_H_
