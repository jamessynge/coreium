#ifndef MCUCORE_EXTRAS_TEST_TOOLS_EEPROM_TEST_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_EEPROM_TEST_UTILS_H_

// Helpers for reading all of the bytes in the EEPROMClass instance via all of
// the methods it exposes for that. Note that it would be nice if these methods
// worked with const EEPROMClass instances, but the Arduino definition has the
// read-only methods declared as non-const.
//
// Author: james.synge@gmail.com

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "extras/host/eeprom/eeprom.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {

using AddressToValueMap = std::unordered_map<int, uint8_t>;  // NOLINT

// Read all bytes from the EEPROM via the read() method.
std::vector<uint8_t> ReadAllBytes(EEPROMClass& eeprom);

// Read all bytes from the EEPROM via the operator[] method.
std::vector<uint8_t> ReadAllBytesViaSubscript(EEPROMClass& eeprom);

// Reach all values of type T from the EEPROM, starting address zero. If the
// EEPROM size is not an integer multiple of sizeof T, then the bytes at the end
// of the EEPROM will not be read.
template <typename T>
std::vector<T> GetAllValues(EEPROMClass& eeprom) {
  const auto last = eeprom.length() - sizeof(T);
  std::vector<T> result;
  for (int idx = 0; idx <= last; ++idx) {
    T t;
    const auto& tref = eeprom.get(idx, t);
    EXPECT_EQ(&t, &tref);
    result.push_back(t);
  }
  return result;
}

// Read all bytes using the above 3 methods, verifying that they all produce
// the same value and returning the values from one of those methods.
std::vector<uint8_t> ReadAllBytesAllWaysAndVerify(EEPROMClass& eeprom);

// Generate |byte_count| bytes in a deterministic fashion based on the value
// |d|, but stored in an unordered map such that we can write them to the EEPROM
// in a different order (perhaps different every time, depending on the
// implementation of the map).
AddressToValueMap GenerateByteValues(double seed, const size_t byte_count);

// Use EEPROMClass::put to write the provided values to the EEPROM.
void PutValues(const AddressToValueMap& values, EEPROMClass& eeprom);

// Use "random" values from GenerateByteValues to initialize the EEPROM.
void RandomizeEeprom(double seed, EEPROMClass& eeprom);

////////////////////////////////////////////////////////////////////////////////
// We use the following ExpectHasValues functions rather than EXPECT_EQ(a, b) or
// EXPECT_THAT(a, ContainerEq(b)) to ensure we get a more detailed error message
// in the event of a mismatch.

// Verify (~= EXPECT_EQ) that the EEPROM holds the expected values. Expected
// must have the same number of bytes as the EEPROM.
void ExpectHasValues(EEPROMClass& eeprom, const std::vector<uint8_t>& expected);

// Verify (~= EXPECT_EQ) that |actual| equals |expected|.
void ExpectHasValues(const std::vector<uint8_t>& actual,
                     const std::vector<uint8_t>& expected);

// Verify that the <address, value> pairs in expected are in the EEPROM.
// Expected does not need to have as many elements as the EEPROM has bytes.
void ExpectHasValues(EEPROMClass& eeprom, const AddressToValueMap& expected);

// Verify that the <address, value> pairs in |expected| are in |actual|.
// Expected does not need to have as many elements as actual.
void ExpectHasValues(const std::vector<uint8_t>& actual,
                     const AddressToValueMap& expected);

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_EEPROM_TEST_UTILS_H_
