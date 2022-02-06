#include "extras/test_tools/eeprom_test_utils.h"

#include <cstdint>
#include <map>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {

// Helpers for reading all of the bytes in the EEPROMClass instance via all of
// the methods it exposes for that. Note that it would be nice if these methods
// worked with const EEPROMClass instances, but the Arduino definition has the
// read-only methods declared as non-const.

std::vector<uint8_t> ReadAllBytes(EEPROMClass& eeprom) {
  std::vector<uint8_t> result;
  result.reserve(eeprom.length());
  for (int idx = 0; idx < eeprom.length(); ++idx) {
    result.push_back(eeprom.read(idx));
  }
  return result;
}

std::vector<uint8_t> ReadAllBytesViaSubscript(EEPROMClass& eeprom) {
  std::vector<uint8_t> result;
  result.reserve(eeprom.length());
  for (int idx = 0; idx < eeprom.length(); ++idx) {
    result.push_back(eeprom[idx]);
  }
  return result;
}

std::vector<uint8_t> ReadAllBytesAllWaysAndVerify(EEPROMClass& eeprom) {
  const auto result = ReadAllBytes(eeprom);
  EXPECT_EQ(result, ReadAllBytesViaSubscript(eeprom));
  EXPECT_EQ(result, GetAllValues<uint8_t>(eeprom));
  return result;
}

AddressToValueMap GenerateByteValues(double seed, const size_t byte_count) {
  // Come up with some values to write deterministically, but we'll store them
  // in an unordered_map so that we'll write them in a different order.
  AddressToValueMap values;
  for (int address = 0; address < byte_count; ++address) {
    seed += seed * address;
    // Choose one of the bytes of d as the next to write to the EEPROM.
    uint8_t b = *(reinterpret_cast<const uint8_t*>(&seed) + 2);
    EXPECT_EQ(values.insert({address, b}).second, true);
  }
  return values;
}

void PutValues(const AddressToValueMap& values, EEPROMClass& eeprom) {
  for (const auto [address, value] : values) {
    eeprom.put(address, value);
    EXPECT_EQ(eeprom.read(address), value);
  }
}

void RandomizeEeprom(double seed, EEPROMClass& eeprom) {
  PutValues(GenerateByteValues(seed, eeprom.length()), eeprom);
}

void ExpectHasValues(EEPROMClass& eeprom,
                     const std::vector<uint8_t>& expected) {
  ExpectHasValues(ReadAllBytesAllWaysAndVerify(eeprom), expected);
}

void ExpectHasValues(const std::vector<uint8_t>& actual,
                     const std::vector<uint8_t>& expected) {
  ASSERT_GE(actual.size(), expected.size());
  EXPECT_EQ(actual.size(), expected.size());

  std::vector<std::string> errors;
  for (size_t idx = 0; idx < actual.size(); ++idx) {
    if (actual[idx] != expected[idx]) {
      errors.push_back(absl::StrFormat("[%d] %d != %d ('%c' != '%c')", idx,
                                       actual[idx], expected[idx], actual[idx],
                                       expected[idx]));
    }
  }

  if (errors.empty()) {
    return;
  }

  ADD_FAILURE() << "actual and expected do not match:\n"
                << absl::StrJoin(errors, "\n");
}

void ExpectHasValues(EEPROMClass& eeprom, const AddressToValueMap& expected) {
  ExpectHasValues(ReadAllBytesAllWaysAndVerify(eeprom), expected);
}

void ExpectHasValues(const std::vector<uint8_t>& actual,
                     const AddressToValueMap& expected) {
  ASSERT_GE(actual.size(), expected.size());
  std::vector<std::string> errors;
  std::map<int, uint8_t> ordered_expected(expected.begin(), expected.end());
  for (const auto [idx, expected_value] : ordered_expected) {
    if (0 <= idx && idx < actual.size()) {
      if (actual[idx] != expected_value) {
        errors.push_back(absl::StrFormat("[%d] %d != %d ('%c' != '%c')", idx,
                                         actual[idx], expected_value,
                                         actual[idx], expected_value));
      }
    } else {
      errors.push_back(absl::StrFormat("[%d] %d != %d ('%c' != '%c')", idx,
                                       actual[idx], expected_value, actual[idx],
                                       expected_value));
    }
  }

  if (errors.empty()) {
    return;
  }

  ADD_FAILURE() << "actual doesn't contain the values in expected:\n"
                << absl::StrJoin(errors, "\n");
}

}  // namespace test
}  // namespace mcucore
