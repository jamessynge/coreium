#include "status.h"

#include "counting_print.h"
#include "hex_escape.h"
#include "mcucore_platform.h"
#include "o_print_stream.h"
#include "progmem_string_data.h"

namespace mcucore {

size_t Status::printTo(Print& out) const {
  CountingPrint counter(out);
  OPrintStream strm(counter);
  if (ok()) {
    strm << MCU_FLASHSTR("OK");
  } else {
    strm << MCU_FLASHSTR("{.code=") << code_;
    if (message_.size()) {
      strm << MCU_FLASHSTR(", .message=") << HexEscaped(message_);
    }
    strm << '}';
  }
  return counter.count();
}

bool operator==(const Status& a, const Status& b) {
  return a.code() == b.code() && a.message() == b.message();
}

}  // namespace mcucore
