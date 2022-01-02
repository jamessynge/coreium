#ifndef MCUCORE_SRC_EEPROM_IO_H_
#define MCUCORE_SRC_EEPROM_IO_H_

// Support for writing names and values to EEPROM, and later reading them back.
// Requires the Arduino EEPROM library.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "progmem_string_view.h"

namespace mcucore {

// Class for computing a 32-bit Cyclic Redundancy Check (a hash).
// Used for verifying that the EEPROM is uncorrupted.
class Crc32 {
 public:
  // Add the next byte of the sequence on which we're computing a CRC.
  void appendByte(uint8_t v);

  // The current value of the CRC.
  uint32_t value() const { return value_; }

  // Store the CRC (value_) at the specified address. Returns the address after
  // the stored CRC.
  int put(int crcAddress) const;

  // Validate that the computed CRC (value_) matches the CRC stored at the
  // specified address.
  bool verify(int crcAddress) const;

 private:
  uint32_t value_ = ~0L;
};

namespace eeprom_io {

// Write name to EEPROM starting at byte toAddress. Does NOT write the
// terminating null.
int SaveName(int toAddress, const char* name);
int SaveName(int toAddress, const ProgmemStringView& name);

// Verify that EEPROM contains name starting at byte atAddress. If matches,
// writes the address of the EEPROM location after the name to afterAddress
// (i.e. the location that would correspond to the terminating null of name, if
// that had been written to EEPROM).
bool VerifyName(int atAddress, const char* name, int* afterAddress);
bool VerifyName(int atAddress, const ProgmemStringView& name,
                int* afterAddress);

// By passing all of the bytes written to a CRC instance as we save to the
// EEPROM, we can ensure that the CRC value is computed from the same bytes
// that we're later going to validate.
void PutBytes(int address, const uint8_t* src, size_t numBytes, Crc32* crc);

// Similarly, we can validate during restore.
void GetBytes(int address, size_t numBytes, uint8_t* dest, Crc32* crc);

}  // namespace eeprom_io
}  // namespace mcucore

#endif  // MCUCORE_SRC_EEPROM_IO_H_
