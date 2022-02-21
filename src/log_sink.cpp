#include "log_sink.h"

#include <stdlib.h>

#include "mcucore_platform.h"
#include "progmem_string_data.h"

#ifndef ARDUINO
#include "base/logging_extensions.h"
#include "extras/test_tools/print_to_std_string.h"  // pragma: keep extras include
#include "glog/logging.h"
#endif  // !ARDUINO

#ifdef ARDUINO
#define DEFAULT_SINK_OUT ::Serial
#else
#define DEFAULT_SINK_OUT ::ToStdErr
#endif

namespace mcucore {
namespace {
#if MCU_HOST_TARGET

Print* the_print_for_log_sink = &(DEFAULT_SINK_OUT);
inline Print& GetPrintForLogSink() { return *the_print_for_log_sink; }

Print* the_print_for_check_sink = &(DEFAULT_SINK_OUT);
inline Print& GetPrintForCheckSink() { return *the_print_for_check_sink; }

CheckSinkExitFn* the_check_sink_exit_fn = nullptr;
inline void CheckSinkExit(std::string_view message) {
  if (the_check_sink_exit_fn != nullptr) {
    (*the_check_sink_exit_fn)(message);
  } else {
    LOG(FATAL) << message;
  }
}

#else  // !MCU_HOST_TARGET

inline Print& GetPrintForLogSink() { return DEFAULT_SINK_OUT; }
inline Print& GetPrintForCheckSink() { return DEFAULT_SINK_OUT; }

#endif  // MCU_HOST_TARGET
}  // namespace

#if MCU_HOST_TARGET

void SetPrintForLogSink(Print* out) {
  the_print_for_log_sink = out != nullptr ? out : &(DEFAULT_SINK_OUT);
}
void SetPrintForCheckSink(Print* out) {
  the_print_for_check_sink = out != nullptr ? out : &(DEFAULT_SINK_OUT);
}
void SetCheckSinkExitFn(CheckSinkExitFn exit_fn) {
  if (the_check_sink_exit_fn != nullptr) {
    delete the_check_sink_exit_fn;
    the_check_sink_exit_fn = nullptr;
  }
  if (exit_fn) {
    the_check_sink_exit_fn = new CheckSinkExitFn(exit_fn);
  }
}

#endif  // MCU_HOST_TARGET

MessageSinkBase::MessageSinkBase(Print& out, const __FlashStringHelper* file,
                                 uint16_t line_number)
    : OPrintStream(out), file_(file), line_number_(line_number) {}

void MessageSinkBase::PrintLocation(Print& out) const {
  if (file_ != nullptr && out.print(file_) > 0) {
    if (line_number_ != 0) {
      out.print(':');
      out.print(line_number_);
    }
    out.print(']');
    out.print(' ');
  }
}

LogSink::LogSink(Print& out, const __FlashStringHelper* file,
                 uint16_t line_number)
    : MessageSinkBase(out, file, line_number) {
  PrintLocation(out_);
}

LogSink::LogSink(const __FlashStringHelper* file, uint16_t line_number)
    : LogSink(GetPrintForLogSink(), file, line_number) {}

LogSink::LogSink(Print& out) : LogSink(out, nullptr, 0) {}

LogSink::LogSink() : LogSink(GetPrintForLogSink()) {}

LogSink::~LogSink() {
  // End the line of output produced by the active logging statement.
  out_.println();
  out_.flush();
}

CheckSink::CheckSink(const __FlashStringHelper* file, uint16_t line_number,
                     const __FlashStringHelper* expression_message)
    : MessageSinkBase(GetPrintForCheckSink(), file, line_number),
      expression_message_(expression_message) {
  Announce(out_);
}

CheckSink::~CheckSink() {
  // End the line of output produced by the current TAS_*CHECK* statement.
  out_.println();
  out_.flush();

#ifdef ARDUINO
  uint8_t seconds = 0;
  while (true) {
    if (seconds < 255) {
      ++seconds;
    }
    delay(10000L + 10000L * seconds);
    Announce(out_);
    out_.println();
    out_.flush();
  }
#else  // !ARDUINO
#if MCU_HOST_TARGET
  FlushLogFiles(base_logging::INFO);
  mcucore::test::PrintToStdString ptss;
  Announce(ptss);
  CheckSinkExit(ptss.str());
#else  // !MCU_HOST_TARGET
#error "Not yet implemented"
#endif  // MCU_HOST_TARGET
#endif  // ARDUINO
}

void CheckSink::Announce(Print& out) const {
  out.print(MCU_FLASHSTR("MCU_CHECK FAILED: "));
  PrintLocation(out);
  if (expression_message_ != nullptr) {
    out.print(expression_message_);
    out.print(' ');
  }
}

}  // namespace mcucore
