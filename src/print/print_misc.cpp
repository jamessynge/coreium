#include "print/print_misc.h"

#include "mcucore_platform.h"
#include "print/counting_print.h"
#include "strings/progmem_string_data.h"

namespace mcucore {

size_t PrintUnknownEnumValueTo(const __FlashStringHelper* name, uint32_t v,
                               Print& out) {
  CountingPrint counter(out);
  counter.print(MCU_FLASHSTR("Undefined "));
  counter.print(name);
  counter.print(' ');
  counter.print('(');
  counter.print(v);
  counter.print(')');
  return counter.count();
}

size_t PrintFlashStringOfLength(const __FlashStringHelper* const ptr,
                                size_t length, Print& out) {
  char buffer[32];
  const char* next = reinterpret_cast<const char*>(ptr);
  size_t remaining = length;
  size_t total = 0;
  while (remaining > sizeof buffer) {
    memcpy_P(buffer, next, sizeof buffer);
    total += out.write(buffer, sizeof buffer);
    next += sizeof buffer;
    remaining -= sizeof buffer;
  }
  if (remaining > 0) {
    memcpy_P(buffer, next, remaining);
    total += out.write(buffer, remaining);
  }
  return total;
}

}  // namespace mcucore
