#include "hash/crc32.h"

#include "log/log.h"

namespace mcucore {

namespace {
// Values from https://www.arduino.cc/en/Tutorial/EEPROMCrc (see also
// mcucore/extras/dev_tools/crc32_table_generator.cc):
constexpr uint32_t kCrcTable[16] AVR_PROGMEM = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
    0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

inline uint32_t GetCrcTableEntry(uint32_t key) {
  auto offset = key & 0x0f;  // One of the 16 entries in the table.
  auto ptr = &kCrcTable[offset];
#ifdef ARDUINO_ARCH_AVR
  // Unable to get the far pointer support to work for some reason (i.e. the
  // Arduino IDE compiler claims that 'pgm_read_dword_far' )
  return pgm_read_dword_near(ptr);
#else   // !ARDUINO_ARCH_AVR
  return *ptr;
#endif  // ARDUINO_ARCH_AVR
}

}  // namespace

void Crc32::appendByte(uint8_t v) {
  MCU_VLOG(6) << MCU_PSD("Crc32::appendByte(") << (v + 0)
              << MCU_PSD(") old value=") << BaseHex << value_;
  value_ = GetCrcTableEntry(value_ ^ v) ^ (value_ >> 4);
  value_ = GetCrcTableEntry(value_ ^ (v >> 4)) ^ (value_ >> 4);
  value_ = ~value_;
  MCU_VLOG(6) << MCU_PSD("new value=") << BaseHex << value_;
}

}  // namespace mcucore
