#include "extras/host/eeprom/eeprom.h"

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "gtest/gtest.h"

namespace {

// Helpers for reading all of the bytes in the EEPROMClass instance via all of
// the methods it exposes for that. Note that it would be nice if these methods
// worked with const EEPROMClass instances, but the Arduino definition has the
// read-only methods declared as non-const.

auto ReadAllBytes(EEPROMClass& eeprom) {
  std::vector<uint8_t> result;
  for (int idx = 0; idx < eeprom.length(); ++idx) {
    result.push_back(eeprom.read(idx));
  }
  return result;
}

auto ReadAllBytesViaSubscript(EEPROMClass& eeprom) {
  std::vector<uint8_t> result;
  for (int idx = 0; idx < eeprom.length(); ++idx) {
    result.push_back(eeprom[idx]);
  }
  return result;
}

// If the EEPROM size is not an integer multiple of sizeof T, then the bytes at
// the end of the EEPROM will not be read.
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

auto ReadAllBytesAllWaysAndVerify(EEPROMClass& eeprom) {
  const auto result = ReadAllBytes(eeprom);
  EXPECT_EQ(result, ReadAllBytesViaSubscript(eeprom));
  EXPECT_EQ(result, GetAllValues<uint8_t>(eeprom));
  return result;
}

auto GenerateByteValues(double d) {
  // Come up with some values to write deterministically, but we'll store them
  // in an unordered_map so that we'll write them in a different order.
  std::unordered_map<int, uint8_t> values;  // NOLINT
  for (int address = 0; address < 1024; ++address) {
    d = d + d * address;
    // Choose one of the bytes of d as the next to write to the EEPROM.
    uint8_t b = *(reinterpret_cast<const uint8_t*>(&d) + 2);
    EXPECT_EQ(values.insert({address, b}).second, true);
  }
  return values;
}

class EepromTest : public testing::Test {
 protected:
  EepromTest() {}
  void SetUp() override {}
  EEPROMClass eeprom_;
};

// I've no idea of the normal starting value of EEPROM cells, but the host
// (fake) version of EEPROMClass initializes to zero. Let's make sure.
TEST_F(EepromTest, StartsZeroedOut) {
  static_assert(sizeof(EEPROMClass) >= 1024, "Wrong size");
  EXPECT_EQ(eeprom_.length(), 1024);

  const std::vector<uint8_t> zeroes(eeprom_.length());
  EXPECT_EQ(zeroes.size(), 1024);
  for (int address = 0; address < 1024; ++address) {
    EXPECT_EQ(zeroes[address], 0);
  }

  EXPECT_EQ(ReadAllBytesAllWaysAndVerify(eeprom_), zeroes);
  EEPROM = eeprom_;
  EXPECT_EQ(ReadAllBytesAllWaysAndVerify(EEPROM), zeroes);
}

TEST_F(EepromTest, WritesBytes) {
  // Write values in the order determined by a hash map.
  std::unordered_map<int, uint8_t> values =  // NOLINT
      GenerateByteValues(3.14159265358979323846);

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
  std::unordered_map<int, uint8_t> values =  // NOLINT
      GenerateByteValues(1.41421356237309504880);

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
  std::unordered_map<int, uint8_t> values =  // NOLINT
      GenerateByteValues(2.71828182845904523536);

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
