#include "log/log_sink.h"

#include "mcucore_platform.h"
#include "strings/progmem_string.h"

#ifdef ARDUINO_ARCH_AVR
#include "platform/avr/watchdog.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// As of Early 2022, Arduino is still based on C++ 2011. When compiling for the
// HOST as a target (i.e. for testing), we use -Wpre-c++14-compat to warn if the
// code uses features added in C++ 2014, or later. However, in that case we also
// want to take advantage of some libraries not available for the Arduino, such
// as Abseil's logging and STL. So, when not compiling for Arduino, we need to
// disable the pre-c++14-compat warning so that we can include such libraries.
#ifndef ARDUINO
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpre-c++14-compat"
#endif

#include <string_view>  // pragma: keep standard include

#include "absl/log/log.h"
#include "absl/log/log_sink_registry.h"  // Provides FlushLogSinks

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif  // !ARDUINO
////////////////////////////////////////////////////////////////////////////////

#ifdef ARDUINO
#define DEFAULT_SINK_OUT ::Serial
#else
#define DEFAULT_SINK_OUT ::ToStdErr
#endif

namespace mcucore {
namespace {
void PrintLocation(Print& out, ProgmemString file, const uint16_t line_number) {
  if (file != nullptr && file.printTo(out) > 0) {
    if (line_number != 0) {
      out.print(':');
      out.print(line_number);
    }
    out.print(']');
    out.print(' ');
  }
}
}  // namespace

#if MCU_HOST_TARGET

namespace {
Print* the_print_for_log_sink = nullptr;
inline Print& GetPrintForLogSink() {
  return the_print_for_log_sink == nullptr ? DEFAULT_SINK_OUT
                                           : *the_print_for_log_sink;
}

Print* the_print_for_check_sink = nullptr;
inline Print& GetPrintForCheckSink() {
  return the_print_for_check_sink == nullptr ? DEFAULT_SINK_OUT
                                             : *the_print_for_check_sink;
}

CheckSinkExitFn* the_check_sink_exit_fn = nullptr;
inline void CheckSinkExit(std::string_view message) {
  if (the_check_sink_exit_fn != nullptr) {
    (*the_check_sink_exit_fn)(message);
  } else {
    // By default, we assume that we can use Abseil's logging features on host,
    // and that it is easiest to debug a failure if all the log files have been
    // flushed before we trigger a crash.
    absl::FlushLogSinks();
    LOG(FATAL) << message;  // COV_NF_LINE
  }
}
}  // namespace

void SetPrintForLogSink(Print* out) { the_print_for_log_sink = out; }
void SetPrintForCheckSink(Print* out) { the_print_for_check_sink = out; }
void SetCheckSinkExitFn(CheckSinkExitFn exit_fn) {
  if (the_check_sink_exit_fn != nullptr) {
    delete the_check_sink_exit_fn;
    the_check_sink_exit_fn = nullptr;
  }
  if (exit_fn) {
    the_check_sink_exit_fn = new CheckSinkExitFn(exit_fn);
  }
}

#else  // !MCU_HOST_TARGET

namespace {
inline Print& GetPrintForLogSink() { return DEFAULT_SINK_OUT; }
inline Print& GetPrintForCheckSink() { return DEFAULT_SINK_OUT; }
}  // namespace

#endif  // MCU_HOST_TARGET

LogSink::LogSink(ProgmemString file, uint16_t line_number)
    : OPrintStream(GetPrintForLogSink()) {
  PrintLocation(out_, file, line_number);
}

LogSink::LogSink() : OPrintStream(GetPrintForLogSink()) {}

LogSink::~LogSink() {
  // End the line of output produced by the active logging statement.
  out_.println();
  out_.flush();
}

CheckSink::CheckSink(ProgmemString file, uint16_t line_number,
                     ProgmemString expression_message)
    : OPrintStream(GetPrintForCheckSink()) {
  DoPrint(MCU_PSD("MCU_CHECK FAILED: "));
  PrintLocation(out_, file, line_number);
  if (expression_message != nullptr) {
    DoPrint(expression_message);
    DoPrint(' ');
  }
}

CheckSink::~CheckSink() {
  // End the line of output produced by the active check statement.
  out_.println();
  out_.flush();

#ifdef ARDUINO_ARCH_AVR
  // Use the watchdog timer to reset the Arduino in a little while so that human
  // intervention isn't required to "recover"; i.e. reboot/restart and try
  // again.
  avr::EnableWatchdogResetMode(4);
  while (true) {
    // Do nothing
  }
#elif MCU_HOST_TARGET
  CheckSinkExit("MCU_CHECK FAILED");
#else
#error "Not yet implemented"
#endif
}

}  // namespace mcucore
