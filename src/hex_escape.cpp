#include "hex_escape.h"

#include "logging.h"

namespace mcucore {

constexpr char kHexDigits[] AVR_PROGMEM = "0123456789ABCDEF";

inline bool IsHexDigit(char c) {
  if ('0' <= c && c <= '9') {
    return true;
  }
  // lc_c is lower case c IFF c was originally upper case.
  c |= static_cast<char>(0x20);
  if ('a' <= c && c <= 'f') {
    return true;
  }
  return false;
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
          // characters. If that is important, modify the #if around this.
          if (old_state == EHexEscapingState::kQuestionMarkOutput) {
            total += out.print('\\');
          }
          total += out.print('?');
          state = EHexEscapingState::kQuestionMarkOutput;
#endif
        } else if (old_state == EHexEscapingState::kHexDigitOutput &&
                   IsHexDigit(c)) {
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
    total += out.print(kHexDigits[(c >> 4) & 0xf]);
    total += out.print(kHexDigits[c & 0xf]);
    state = EHexEscapingState::kHexDigitOutput;
  }
  return total;
}

size_t PrintCharHexEscaped(Print& out, const char c) {
  EHexEscapingState state;
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

}  // namespace mcucore
