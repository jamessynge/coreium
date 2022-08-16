#ifndef MCUCORE_SRC_EEPROM_EEPROM_IO_H_
#define MCUCORE_SRC_EEPROM_EEPROM_IO_H_

// Support for writing values to EEPROM, and later reading them back, with
// verification that values match some expectation. Requires the Arduino EEPROM
// library.
//
// Author: james.synge@gmail.com

#include "hash/crc32.h"
#include "mcucore_platform.h"
#include "strings/progmem_string_view.h"

namespace mcucore {
namespace eeprom_io {

// Write name to EEPROM starting at byte toAddress. Does NOT write the
// terminating null. Returns the EEPROM address after the last byte to which
// name was written.
int SaveName(int toAddress, const char* name);
int SaveName(int toAddress, const ProgmemStringView& name);

// Verify that EEPROM contains name starting at byte atAddress. If matches,
// writes the address of the EEPROM location after the name to afterAddress
// (i.e. the location that would correspond to the terminating null of name, if
// that had been written to EEPROM). name must not be null.
bool VerifyName(int atAddress, const char* name, int* afterAddress);
bool VerifyName(int atAddress, const ProgmemStringView& name,
                int* afterAddress);

// Writes numBytes from RAM, starting at src, to EEPROM starting at address. If
// crc is not null, each byte is also appended to the CRC, thus allowing
// verification that bytes read back from EEPROM haven't been corrupted. src
// must not be null, crc may be null.
void PutBytes(int address, const uint8_t* src, size_t numBytes, Crc32* crc);

// Similarly, we can validate during restore. dest must not be null.
void GetBytes(int address, size_t numBytes, uint8_t* dest, Crc32* crc);

// Store the CRC (i.e. crc.value()) at the specified address. Returns the
// address after the stored CRC.
int PutCrc(int toAddress, const Crc32& crc);

// Returns true if the computed CRC (i.e. crc.value()) matches the CRC stored at
// the specified address.
bool VerifyCrc(int atAddress, const Crc32& crc);

}  // namespace eeprom_io
}  // namespace mcucore

#endif  // MCUCORE_SRC_EEPROM_EEPROM_IO_H_
