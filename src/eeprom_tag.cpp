#include "eeprom_tag.h"

#include "mcucore_platform.h"

// We reserve domain 0 and domain 255 because bytes in the the EEPROM are set
// to one of those values when cleared/erased, but it isn't clear which one, and
// historically different EEPROM designs have used one or the other of those two
// values. Avoiding them allows us to double check that stored domains are
// valid. For Microchip AVR microcontrollers, it seems most likely that 255
// 0xFF) is the cleared value.
MCU_DEFINE_DOMAIN(0);
MCU_DEFINE_DOMAIN(255);

namespace mcucore {

bool IsReservedDomain(const EepromDomain domain) {
  return domain == MCU_DOMAIN(0) || domain == MCU_DOMAIN(255);
}

// bool EepromTag::IsUnused() const { return domain.value() == 0 && id == 255; }

bool operator==(const EepromTag& lhs, const EepromTag& rhs) {
  return lhs.domain == rhs.domain && lhs.id == rhs.id;
}

void EepromTag::InsertInto(OPrintStream& strm) const {
  strm << MCU_FLASHSTR("{.domain=") << domain.value() << MCU_FLASHSTR(", .id=")
       << id << '}';
}

}  // namespace mcucore
