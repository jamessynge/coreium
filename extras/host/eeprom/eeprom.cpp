#include "extras/host/eeprom/eeprom.h"

#include "glog/logging.h"

EEPROMClass::EEPROMClass(uint16_t length) : data_(length, 0) {
  CHECK_NE(length, 0);
}

EEPROMClass EEPROM;  // NOLINT
