#include "json/json_encoder_helpers.h"

#include "log/log.h"
#include "mcucore_platform.h"
#include "print/counting_print.h"

namespace mcucore {

size_t PrintableJsonObject::printTo(Print& out) const {
  CountingPrint counter(out);
  JsonObjectEncoder::Encode(source_, counter);
#if SIZE_MAX < UINT32_MAX
  MCU_DCHECK_LE(counter.count(), SIZE_MAX)
      << MCU_PSD("size_t max (") << SIZE_MAX << MCU_PSD(") is too small for ")
      << counter.count();
#endif
  return counter.count();
}

}  // namespace mcucore
