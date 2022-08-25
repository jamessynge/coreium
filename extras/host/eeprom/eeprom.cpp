#include "extras/host/eeprom/eeprom.h"

#include "absl/log/check.h"

namespace internal {

uint8_t eeprom_read_byte(EEPROMClass& eeprom, int index) {
  return eeprom.read(index);
}

void eeprom_write_byte(EEPROMClass& eeprom, int index, uint8_t value) {
  eeprom.write(index, value);
}

}  // namespace internal

EEPROMClass::EEPROMClass(uint16_t length) : data_(length, 0) {
  CHECK_NE(length, 0);
}
EEPROMClass EEPROM;  // NOLINT
