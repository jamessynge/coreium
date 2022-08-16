#ifndef MCUCORE_SRC_MISC_UUID_H_
#define MCUCORE_SRC_MISC_UUID_H_

#include "eeprom/eeprom_tag.h"
#include "eeprom/eeprom_tlv.h"
#include "mcucore_platform.h"
#include "status/status.h"

namespace mcucore {

class Uuid {
 public:
  static constexpr uint_fast8_t kNumBytes = 16;

  void Zero();
  void Generate();

  // Read 16 bytes from the entry identified by `tag` in the EEPROM managed by
  // `tlv`. Returns an error if it fails, else OK.
  Status ReadFromEeprom(EepromTlv& tlv, EepromTag tag);

  // Write data_ into the EEPROM identified by `tag`, replacing any previous
  // entry with the same tag. Returns an error if it fails, else OK.
  Status WriteToEeprom(EepromTlv& tlv, EepromTag tag) const;

  // Read from EEPROM. If not present, generate a value and store
  // in EEPROM, then return OK. Returns an error if entry is found but can't be
  // read, or if we can't store the generated value in the EEPROM.
  Status ReadOrStoreEntry(EepromTlv& tlv, EepromTag tag);

  // Print in standard UUID format, five groups of hexadecimal characters,
  // separated by hyphens, in the form 8-4-4-4-12 (i.e. 36 characters).
  size_t printTo(Print& out) const;

  // Compares two Uuids for equality.
  friend bool operator==(const Uuid& a, const Uuid& b);

  template <int N>
  void SetForTest(const uint8_t (&data)[N]) {
    static_assert(N == 16);
    for (uint_fast8_t ndx = 0; ndx < 16; ++ndx) {
      data_[ndx] = data[ndx];
    }
  }

 private:
  uint8_t data_[16];
};

inline bool operator!=(const Uuid& a, const Uuid& b) { return !(a == b); }

}  // namespace mcucore

#endif  // MCUCORE_SRC_MISC_UUID_H_
