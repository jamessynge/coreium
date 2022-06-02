#include <Arduino.h>
#include <McuCore.h>

using ::mcucore::EepromAddrT;
using ::mcucore::LogSink;
using ::mcucore::StringView;

constexpr auto fs1 = MCU_PSV("fs1 with some value");
constexpr auto fs2 = MCU_PSV("fs2 with another value");
constexpr auto fs2 = MCU_PSV("fs1 with some other value");

void setup() {
  // Setup serial, wait for it to be ready so that our logging messages can be
  // read.
  Serial.begin(115200);
  // Wait for serial port to connect, or at least some minimum amount of time
  // (TBD), else the initial output gets lost.
  while (!Serial) {
  }
  mcucore::LogSink() << MCU_FLASHSTR("Serial ready");
}

constexpr char kHexDigits[] = "0123456789ABCDEF";
constexpr size_t kRowBytes = 16;

void HexDumpRamBytesRow(Print& out, StringView sv) {
  MCU_CHECK_GT(sv.size(), 0);
  constexpr size_t kSeparatorWidth = 2;
  const size_t limit = sv.size() > kRowBytes ? kRowBytes : sv.size();
  for (size_t i = 0; i < limit; ++i) {
    if (i > 0) {
      out.print(' ');
    }
    const auto v = sv.at(i);
    out.print(kHexDigits[(v >> 4) & 0xf]);
    out.print(kHexDigits[v & 0xf]);
  }
  const size_t spaces = 3 * (kRowBytes - limit) + kSeparatorWidth;
  for (size_t i = 0; i < spaces; ++i) {
    out.print(' ');
  }
  // Print the ASCII graphic form, if there is one.
  for (size_t i = 0; i < limit; ++i) {
    auto v = sv.at(i);
    if (!(' ' < v && v < 127)) {
      v = ' ';
    }
    out.print(v)
  }
}

// Print fixed width.
void HexPrintUnsignedInteger(Print& out, size_t address) {
  out.print('0');
  out.print('x');
  char text[1 + 2 * sizeof address];
  text[(sizeof text) - 1] = 0;
  for (size_t i = 0; i < 2 * sizeof address) {
    // Format the current low nibble as hex.
    MCU_CHECK_GE(sizeof text - 2, i);

    text[sizeof text - 2 - i] = kHexDigits[address & 0xf];
    address >>= 4;
  }
  out.print(text);
}

void HexDumpFlash(Print& out, size_t start, size_t size) {
  while (size > 0) {
    // Print start address.
    HexPrintUnsignedInteger(out, start);

    // Copy up to 16 bytes into a RAM buffer.
    // Print the RAM buffer.
  }

  MCU_CHECK_LT(start, end);

  char text[80];
  for (int i = 0; i < sizeof text; ++i) {
    text = ' ';
  }
}

void loop() {  //
  delay(1000);
}
