
#include "hex_dump.h"

#include "limits.h"
#include "logging.h"

namespace mcucore {
namespace {

constexpr char kHexDigits[] AVR_PROGMEM = "0123456789ABCDEF";
constexpr uint_fast8_t kRowBytes = 16;
constexpr uint_fast8_t kSeparatorWidth = 2;

char NibbleToAsciiHex(uint8_t v) {
  MCU_DCHECK_LT(v, 16);
  return pgm_read_byte(kHexDigits + v);
}

// Compute number of digits needed for maximum label address.
uint_fast8_t MaxLabelHexDigits(size_t start_address, size_t num_bytes) {
  // TODO implement
  return 4;
}

void HexPrintLabelAddress(Print& out, size_t address, const uint8_t digits) {
  constexpr auto kMaxDigits = 2 * sizeof address;
  out.print('0');
  out.print('x');
  MCU_DCHECK_GE(kMaxDigits, digits);
  char text[kMaxDigits + /*room for NUL*/ 1];
  text[kMaxDigits] = 0;
  for (size_t i = 0; i < kMaxDigits; ++i) {
    // Format the current low nibble as hex.
    MCU_CHECK_GE(sizeof text - 2, i);
    text[sizeof text - 2 - i] = NibbleToAsciiHex(address & 0xf);
    address >>= 4;
    if (i >= digits) {
      MCU_DCHECK_EQ(address, 0);
    }
  }
  char* text_start = &text[kMaxDigits - digits];
  out.print(text_start);
}

}  // namespace

void HexDumpBytes(Print& out, size_t start_address, size_t num_bytes,
                  GetByteFunction get_byte_fn) {
  const auto label_digits = MaxLabelHexDigits(start_address, num_bytes);

  while (num_bytes > 0) {
    // Print one row. Start with the label (the address of the first byte).
    HexPrintLabelAddress(out, start_address, label_digits);
    out.print(MCU_FLASHSTR(": "));

    // How long is that row?
    const uint_fast8_t num_row_bytes =
        (num_bytes > kRowBytes) ? kRowBytes : num_bytes;

    // Print the hex format of those bytes.
    for (uint_fast8_t ndx = 0; ndx < num_row_bytes; ++ndx) {
      out.print(' ');
      const auto v = get_byte_fn(start_address + ndx);
      out.print(NibbleToAsciiHex((v >> 4) & 0xf));
      out.print(NibbleToAsciiHex(v & 0xf));
    }

    // Print the spaces following those hex digits, allowing for the
    // case where there are fewer than kRowBytes bytes.
    uint_fast8_t num_spaces = 3 * (kRowBytes - num_row_bytes) + kSeparatorWidth;
    while (num_spaces > 0) {
      out.print(' ');
      --num_spaces;
    }

    // Print the ASCII graphic form of each byte, if there is one, else a space.
    for (uint_fast8_t ndx = 0; ndx < num_row_bytes; ++ndx) {
      char v = get_byte_fn(start_address + ndx);
      if (!(' ' < v && v < 127)) {
        v = '.';
      }
      out.print(v);
    }

    out.println();

    // Where is the next row?
    start_address += num_row_bytes;
    num_bytes -= num_row_bytes;
  }
}

void HexDumpFlashBytes(Print& out, size_t start_address, size_t num_bytes) {
  HexDumpBytes(out, start_address, num_bytes, [](size_t address) -> uint8_t {
    return pgm_read_byte(address);
  });
}

namespace {
EEPROMClass* eeprom_ptr = nullptr;

uint8_t GetEepromBytes(size_t address) {
  MCU_CHECK_NE(eeprom_ptr, nullptr);
  return eeprom_ptr->read(address);
}
}  // namespace

void HexDumpEepromBytes(Print& out, size_t start_address, size_t num_bytes,
                        EEPROMClass& eeprom) {
  MCU_CHECK_EQ(eeprom_ptr, nullptr);
  eeprom_ptr = &eeprom;
  HexDumpBytes(out, start_address, num_bytes, GetEepromBytes);
  MCU_CHECK_EQ(eeprom_ptr, &eeprom);
  eeprom_ptr = nullptr;
}

}  // namespace mcucore
