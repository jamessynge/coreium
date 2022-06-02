#ifndef SRC_HEX_DUMP_H
#define SRC_HEX_DUMP_H

// Support for dumping bytes in the hex dump format. Primarily of interest
// when debugging.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "progmem_string_view.h"

namespace mcucore {

using GetByteFunction = uint8_t (*)(size_t);

// Dump the `num_bytes` bytes provided by `get_byte_fn` from `start_address`.
void HexDumpBytes(Print& out, size_t start_address, size_t num_bytes,
                  GetByteFunction get_byte_fn);

void HexDumpFlashBytes(Print& out, size_t start_address, size_t num_bytes);

inline void HexDumpLeadingFlashBytes(Print& out, size_t num_bytes) {
  HexDumpFlashBytes(out, static_cast<size_t>(0), num_bytes);
}

inline void HexDumpFlashBytes(Print& out, void* start_address,
                              size_t num_bytes) {
  HexDumpFlashBytes(out, reinterpret_cast<uintptr_t>(start_address), num_bytes);
}

inline void HexDumpFlashBytes(Print& out, ProgmemStringView str) {
  HexDumpFlashBytes(out, reinterpret_cast<uintptr_t>(str.progmem_ptr()),
                    str.size());
}

inline void HexDumpFlashBytes(Print& out, __FlashStringHelper* start_address,
                              size_t num_bytes) {
  HexDumpFlashBytes(
      out, reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(start_address)),
      num_bytes);
}

void HexDumpEepromBytes(Print& out, size_t start_address, size_t num_bytes,
                        EEPROMClass& eeprom);

}  // namespace mcucore

#endif  // SRC_HEX_DUMP_H
