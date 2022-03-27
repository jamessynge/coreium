#ifndef MCUCORE_SRC_CRC32_H_
#define MCUCORE_SRC_CRC32_H_

// Class for computing a 32-bit Cyclic Redundancy Check. Used for verifying that
// the EEPROM is uncorrupted. To learn more about these, see:
//
//   * 32-Bit Cyclic Redundancy Codes for Internet Applications
//     https://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.11.8323
//
//   * Catalogue of parametrised CRC algorithms
//     https://reveng.sourceforge.io/crc-catalogue/all.htm
//
//   * avr-libc util/crc16.h offers inline functions for 16-bit CRC calculation.
//     https://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
//
//   * https://tools.ietf.org/id/draft-ietf-tsvwg-sctpcsum-00.txt
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"

namespace mcucore {

class Crc32 {
 public:
  // Don't change this initial value, unless you want to invalidate all existing
  // CRC values computed from this class.
  static constexpr uint32_t kInitialValue = ~0L;

  Crc32() : value_(kInitialValue) {}
  explicit Crc32(uint32_t initial_value) : value_(initial_value) {}

  // Add the next byte of the sequence on which we're computing a CRC.
  void appendByte(uint8_t v);

  // The current value of the CRC.
  uint32_t value() const { return value_; }

 private:
  uint32_t value_;
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_CRC32_H_
