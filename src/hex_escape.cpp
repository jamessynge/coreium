#include "hex_escape.h"

#include <ctype.h>

#include "logging.h"
#include "mcucore_platform.h"

namespace mcucore {

constexpr char kHexDigits[] AVR_PROGMEM = "0123456789ABCDEF";

char NibbleToAsciiHex(uint8_t v) {
  MCU_DCHECK_LT(v, 16);
  return pgm_read_byte(kHexDigits + v);
}

size_t PrintCharWithStateHexEscaped(Print& out, const char c,
                                    EHexEscapingState& state) {
  const auto old_state = state;
  state = EHexEscapingState::kNormal;
  size_t total = 0;
  switch (0) {
    default:
      if (' ' <= c && c < 127) {
        if (c == '"') {
          total += out.print('\\');
          total += out.print('"');
        } else if (c == '\\') {
          total += out.print('\\');
          total += out.print('\\');
#if MCU_HOST_TARGET
        } else if (c == '?') {
          // C++ 14 and before support trigraphs as a way of representing
          // characters. This is enabled for the host target so that I can copy
          // and paste escaped text into C++ source. If that is important for
          // the embedded target, we've moved beyond C++ 14 for all targets,
          // modify the #if around this as appropriate.
          if (old_state == EHexEscapingState::kQuestionMarkOutput) {
            total += out.print('\\');
          }
          total += out.print('?');
          state = EHexEscapingState::kQuestionMarkOutput;
#endif
        } else if (old_state == EHexEscapingState::kHexDigitOutput &&
                   isxdigit(c)) {
          // Need to hex escape this character.
          break;
        } else {
          total += out.print(c);
        }
        return total;
      }
  }
  // NOTE: Not emitting "Simple Escape Sequences" for any characters other than
  // new line and carriage return. I'm assuming that the strings might contain
  // byte values that the reader wants to understand more easily that doing a
  // lookup for \v or \f.
  if (c == '\n') {
    total += out.print('\\');
    total += out.print('n');
  } else if (c == '\r') {
    total += out.print('\\');
    total += out.print('r');
  } else {
    total += out.print('\\');
    total += out.print('x');
    total += out.print(NibbleToAsciiHex((c >> 4) & 0xf));
    total += out.print(NibbleToAsciiHex(c & 0xf));
    state = EHexEscapingState::kHexDigitOutput;
  }
  return total;
}

size_t PrintCharHexEscaped(Print& out, const char c) {
  EHexEscapingState state = EHexEscapingState::kNormal;
  return PrintCharWithStateHexEscaped(out, c, state);
}

PrintHexEscaped::PrintHexEscaped(Print& wrapped)
    : wrapped_(wrapped), state_(EHexEscapingState::kNormal) {}

size_t PrintHexEscaped::write(uint8_t b) {
  return PrintCharWithStateHexEscaped(wrapped_, static_cast<char>(b), state_);
}

size_t PrintHexEscaped::write(const uint8_t* buffer, size_t size) {
  size_t count = 0;
  for (size_t ndx = 0; ndx < size; ++ndx) {
    count += PrintCharWithStateHexEscaped(
        wrapped_, static_cast<char>(buffer[ndx]), state_);
  }
  return count;
}

size_t PrintWithEthernetFormatting(Print& out, const uint8_t* ptr,
                                   uint8_t num_bytes) {
  size_t result = 0;
  for (uint8_t i = 0; i < num_bytes; ++i) {
    if (i > 0) {
      result += out.print('-');
    }
    auto v = ptr[i];
    result += out.print(NibbleToAsciiHex((v >> 4) & 0xf));
    result += out.print(NibbleToAsciiHex(v & 0xf));
  }
  return result;
}

}  // namespace mcucore
