#ifndef MCUCORE_SRC_UUID_H_
#define MCUCORE_SRC_UUID_H_

#include "eeprom_region.h"
#include "eeprom_tag.h"
#include "eeprom_tlv.h"
#include "mcucore_platform.h"
#include "status.h"

namespace mcucore {

class Uuid {
 public:
  void Zero();
  void Generate();

  Status ReadFromRegion(EepromRegionReader& region);
  Status WriteToRegion(EepromRegion& region) const;

  Status ReadFromEeprom(EepromTlv& tlv, EepromTag tag);
  Status WriteToEeprom(EepromTlv& tlv, EepromTag tag) const;

  // Read from EEPROM. If not present, generate a value and store
  // in EEPROM, then return OK. Returns an error if entry is found but can't be
  // read, or if we can't store the generated value in the EEPROM.
  Status ReadOrStoreEntry(EepromTlv& tlv, EepromTag tag);

  // Print in standard UUID format, five groups of hexadecimal characters,
  // separated by hyphens, in the form 8-4-4-4-12. Requires 36 characters,
  // and maybe a terminating null to represent as a string.
  size_t printTo(Print& out) const;

  friend bool operator==(const Uuid& a, const Uuid& b);

 private:
  uint8_t data_[16];
};

inline bool operator!=(const Uuid& a, const Uuid& b) { return !(a == b); }

}  // namespace mcucore

#endif  // MCUCORE_SRC_UUID_H_
