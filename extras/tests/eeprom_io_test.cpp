#include "eeprom_io.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <string_view>
#include <vector>

#include "crc32.h"
#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/eeprom_test_utils.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "progmem_string_view.h"

namespace mcucore {
namespace eeprom_io {
namespace test {
namespace {

using ::mcucore::PrintValueToStdString;
using ::mcucore::test::ExpectHasValues;
using ::mcucore::test::RandomizeEeprom;
using ::mcucore::test::ReadAllBytesAllWaysAndVerify;

template <class C>
void UpdateCollection(std::string_view str, size_t pos, C& collection) {
  for (const char c : str) {
    collection[pos++] = static_cast<uint8_t>(c);
  }
}

auto Crc32OfBytes(const std::vector<uint8_t>& bytes) {
  Crc32 crc;
  for (const auto v : bytes) {
    crc.appendByte(v);
  }
  return crc.value();
}

class EepromIoTest : public testing::Test {
 protected:
  void SetUp() override {  // Reset the EEPROM to all zeroes.
    EEPROM = EEPROMClass();
  }
};

TEST_F(EepromIoTest, SaveAndVerifyNameCharStr) {
  const char* kName = "Foo";
  RandomizeEeprom(9876.54321, EEPROM);
  auto all_values = ReadAllBytesAllWaysAndVerify(EEPROM);
  const int to_address = 123;
  int after_address = 9999;
  EXPECT_FALSE(VerifyName(to_address, kName, &after_address));
  EXPECT_EQ(after_address, 9999);
  EXPECT_EQ(SaveName(to_address, kName), to_address + 3);
  EXPECT_TRUE(VerifyName(to_address, kName, &after_address));
  EXPECT_EQ(after_address, to_address + 3);

  // Should have only written to 3 bytes.
  UpdateCollection(kName, to_address, all_values);
  EXPECT_EQ(ReadAllBytesAllWaysAndVerify(EEPROM), all_values);
}

TEST_F(EepromIoTest, SaveAndVerifyNameProgmemStringView) {
  const char kName[] = "FooBar";
  const ProgmemStringView name(kName);
  EXPECT_EQ(name.size(), strlen(kName));
  EXPECT_EQ(PrintValueToStdString(name), kName);

  RandomizeEeprom(9876.54321, EEPROM);
  auto all_values = ReadAllBytesAllWaysAndVerify(EEPROM);
  const int to_address = 321;
  int after_address = 9999;
  EXPECT_FALSE(VerifyName(to_address, name, &after_address));
  EXPECT_EQ(after_address, 9999);
  EXPECT_EQ(SaveName(to_address, name), to_address + name.size());
  EXPECT_TRUE(VerifyName(to_address, name, &after_address));
  EXPECT_EQ(after_address, to_address + name.size());

  // Should have only written these bytes, all others should be unchanged (this
  // doesn't actually confirm that the others weren't re-written to the same
  // values).
  UpdateCollection(kName, to_address, all_values);
  ExpectHasValues(EEPROM, all_values);

  // If we modify one of the characters, the name will no longer be verifiable.
  EEPROM.write(to_address + 5, 'R');
  after_address = 5555;
  EXPECT_FALSE(VerifyName(to_address, name, &after_address));
  EXPECT_EQ(after_address, 5555);
}

TEST_F(EepromIoTest, PutAndGetBytesNoCrc32) {
  const std::vector<uint8_t> kAllBytes = {34, 98, 30, 59, 56, 53, 10, 209};
  for (int size : {0, 1, 2, static_cast<int>(kAllBytes.size())}) {
    const std::vector<uint8_t> kBytes(kAllBytes.data(),
                                      kAllBytes.data() + size);
    for (int starting_address : {0, 1, 100, 1000}) {
      RandomizeEeprom(54321.9876, EEPROM);
      PutBytes(starting_address, kBytes.data(), size, nullptr);
      std::vector<uint8_t> bytes(size);
      GetBytes(starting_address, size, bytes.data(), nullptr);
      ExpectHasValues(bytes, kBytes);
    }
  }
}

TEST_F(EepromIoTest, PutAndGetBytesWithCrc32) {
  const std::vector<uint8_t> kAllBytes = {34, 98, 30, 59, 56, 53, 10, 209};
  for (int size : {0, 1, 2, static_cast<int>(kAllBytes.size())}) {
    const std::vector<uint8_t> kBytes(kAllBytes.data(),
                                      kAllBytes.data() + size);
    // For these kBytes, we should always get the same CRC value, regardless of
    // address to which we put it in the EEPROM.
    const auto expected_crc = Crc32OfBytes(kBytes);
    for (int starting_address : {0, 1, 100, 1000}) {
      RandomizeEeprom(54321.9876, EEPROM);
      Crc32 put_crc;
      PutBytes(starting_address, kBytes.data(), size, &put_crc);
      const int crc_address = starting_address + size;
      EXPECT_EQ(PutCrc(crc_address, put_crc), crc_address + 4);
      EXPECT_EQ(put_crc.value(), expected_crc);

      {
        std::vector<uint8_t> bytes(size);
        Crc32 get_crc;
        GetBytes(starting_address, size, bytes.data(), &get_crc);
        EXPECT_TRUE(VerifyCrc(crc_address, get_crc));
        ExpectHasValues(bytes, kBytes);
        EXPECT_EQ(get_crc.value(), expected_crc);
      }

      // If we corrupt the first byte, the CRC will change.
      if (size > 0) {
        EEPROM.write(starting_address, ~kBytes[0]);
        std::vector<uint8_t> bytes(size);
        Crc32 get_crc;
        GetBytes(starting_address, size, bytes.data(), &get_crc);
        EXPECT_FALSE(VerifyCrc(crc_address, get_crc));
        EXPECT_NE(bytes, kBytes);
        EXPECT_NE(get_crc.value(), expected_crc);
      }
    }
  }
}

}  // namespace
}  // namespace test
}  // namespace eeprom_io
}  // namespace mcucore
