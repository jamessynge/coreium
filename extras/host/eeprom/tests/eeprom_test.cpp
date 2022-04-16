#include "extras/host/eeprom/eeprom.h"

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "extras/test_tools/eeprom_test_utils.h"
#include "gtest/gtest.h"

namespace {
using ::mcucore::test::AddressToValueMap;
using ::mcucore::test::GenerateByteValues;
using ::mcucore::test::ReadAllBytesAllWaysAndVerify;

class EepromTest : public testing::Test {
 protected:
  EepromTest() {}
  void SetUp() override {}
  EEPROMClass eeprom_;
};

// I've no idea of the normal starting value of EEPROM cells, but the host
// (fake) version of EEPROMClass initializes to zero. Let's make sure.
TEST_F(EepromTest, StartsZeroedOut) {
  EXPECT_EQ(eeprom_.length(), EEPROMClass::kDefaultSize);

  const std::vector<uint8_t> zeroes(eeprom_.length());
  EXPECT_EQ(zeroes.size(), EEPROMClass::kDefaultSize);
  for (int address = 0; address < EEPROMClass::kDefaultSize; ++address) {
    EXPECT_EQ(zeroes[address], 0);
  }

  EXPECT_EQ(ReadAllBytesAllWaysAndVerify(eeprom_), zeroes);
  EEPROM = eeprom_;
  EXPECT_EQ(ReadAllBytesAllWaysAndVerify(EEPROM), zeroes);
}

TEST_F(EepromTest, WritesBytes) {
  // Write values in the order determined by a hash map.
  AddressToValueMap values =
      GenerateByteValues(3.14159265358979323846, eeprom_.length());

  for (const auto [address, value] : values) {
    EXPECT_EQ(eeprom_.read(address), 0);
    eeprom_.write(address, value);
    EXPECT_EQ(eeprom_.read(address), value);
  }

  const auto stored = ReadAllBytesAllWaysAndVerify(eeprom_);
  for (const auto [address, value] : values) {
    EXPECT_EQ(stored[address], value);
  }
}

TEST_F(EepromTest, UpdateBytes) {
  // Update values in the order determined by a hash map.
  AddressToValueMap values =
      GenerateByteValues(1.41421356237309504880, eeprom_.length());

  for (const auto [address, value] : values) {
    EXPECT_EQ(eeprom_.read(address), 0);
    eeprom_.update(address, value);
    EXPECT_EQ(eeprom_.read(address), value);
  }

  const auto stored = ReadAllBytesAllWaysAndVerify(eeprom_);
  for (const auto [address, value] : values) {
    EXPECT_EQ(stored[address], value);
  }
}

TEST_F(EepromTest, PutBytes) {
  // Put bytes in the order determined by a hash map.
  AddressToValueMap values =
      GenerateByteValues(2.71828182845904523536, eeprom_.length());

  for (const auto [address, value] : values) {
    EXPECT_EQ(eeprom_.read(address), 0);
    eeprom_.put(address, value);
    EXPECT_EQ(eeprom_.read(address), value);
  }

  const auto stored = ReadAllBytesAllWaysAndVerify(eeprom_);
  for (const auto [address, value] : values) {
    EXPECT_EQ(stored[address], value);
  }
}

TEST_F(EepromTest, PutDoubles) {
  // Put doubles in the order determined by a hash map.
  std::unordered_map<int, double> values;  // NOLINT
  std::vector<uint8_t> expected_bytes(eeprom_.length(), 0);
  double d = 2.30258509299404568402;
  for (int address = 0; address + sizeof(double) <= eeprom_.length();
       address += sizeof(double) + 1) {
    d = d + d * address;
    EXPECT_EQ(values.insert({address, d}).second, true);
    std::memcpy(expected_bytes.data() + address, &d, sizeof d);
  }

  for (const auto [address, value] : values) {
    double d;
    EXPECT_EQ(eeprom_.get(address, d), 0);
    eeprom_.put(address, value);
    EXPECT_EQ(eeprom_.get(address, d), value);
  }

  EXPECT_EQ(ReadAllBytesAllWaysAndVerify(eeprom_), expected_bytes);
  for (const auto [address, value] : values) {
    double d;
    EXPECT_EQ(eeprom_.get(address, d), value);
  }
}

}  // namespace
