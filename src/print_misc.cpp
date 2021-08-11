#include "print_misc.h"

#include "counting_print.h"
#include "inline_literal.h"
#include "mcucore_platform.h"

namespace mcucore {

size_t PrintUnknownEnumValueTo(const __FlashStringHelper* name, uint32_t v,
                               Print& out) {
  CountingPrint counter(out);
  counter.print(TAS_FLASHSTR("Undefined "));
  counter.print(name);
  counter.print(' ');
  counter.print('(');
  counter.print(v);
  counter.print(')');
  return counter.count();
}

}  // namespace mcucore
