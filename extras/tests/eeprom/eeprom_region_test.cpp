#include "eeprom/eeprom_region.h"

#include <algorithm>
#include <cstring>
#include <limits>

#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "status/status_code.h"
#include "strings/string_view.h"

namespace mcucore {
namespace test {
namespace {

using ::testing::AllOf;
using ::testing::HasSubstr;

class EepromRegionReaderTest : public testing::Test {
 protected:
  EEPROMClass eeprom_;
};

TEST_F(EepromRegionReaderTest, Unusable) {
  EepromRegionReader region;
  EXPECT_EQ(region.cursor(), 0);
  EXPECT_EQ(region.length(), 0);
  EXPECT_EQ(region.available(), 0);
  EXPECT_THAT(PrintValueToStdString(region),
              AllOf(HasSubstr(".cursor=0"), HasSubstr(".length=0"),
                    HasSubstr(".available=0")));
}

TEST_F(EepromRegionReaderTest, CreateAndCopy) {
  auto validator = [](EepromRegionReader& region,
                      const EepromAddrT start_address,
                      const EepromAddrT region_length) {
    EXPECT_EQ(region.start_address(), start_address);
    EXPECT_EQ(region.length(), region_length);
    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.available(), region_length);

    auto copy = region;

    EXPECT_EQ(copy.start_address(), start_address);
    EXPECT_EQ(copy.length(), region_length);
    EXPECT_EQ(copy.cursor(), 0);
    EXPECT_EQ(copy.available(), region_length);

    EXPECT_TRUE(region.set_cursor(1));

    EXPECT_EQ(region.start_address(), start_address);
    EXPECT_EQ(region.cursor(), 1);
    EXPECT_EQ(copy.start_address(), start_address);
    EXPECT_EQ(copy.cursor(), 0);

    region = copy;

    EXPECT_EQ(region.start_address(), start_address);
    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(copy.start_address(), start_address);
    EXPECT_EQ(copy.cursor(), 0);

    region.Invalidate();

    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.length(), 0);
    EXPECT_EQ(region.available(), 0);

    region = copy;

    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.length(), region_length);
    EXPECT_EQ(region.available(), region_length);
  };

  {
    EepromRegionReader region(eeprom_, 0, eeprom_.length());
    validator(region, 0, eeprom_.length());
  }

  {
    EepromRegionReader region(eeprom_, 10, 100);
    validator(region, 10, 100);
  }

  {
    EepromRegionReader region(eeprom_, 10, 1);
    validator(region, 10, 1);
  }
}

class EepromRegionTest : public EepromRegionReaderTest {
 protected:
};

TEST_F(EepromRegionTest, Unusable) {
  EepromRegion region;
  EXPECT_EQ(region.cursor(), 0);
  EXPECT_EQ(region.length(), 0);
  EXPECT_EQ(region.available(), 0);
}

template <typename T>
void VerifyNoWriteNoRead(EepromRegion& region, const T value) {
  for (EepromAddrT available = 0; available < sizeof(T); ++available) {
    ASSERT_GE(region.length(), available);
    ASSERT_TRUE(region.set_cursor(region.length() - available));
    ASSERT_EQ(region.available(), available);

    ASSERT_FALSE(region.Write<T>(value));
    ASSERT_EQ(region.available(), available);

    T t;
    ASSERT_FALSE(region.ReadInto(t));
    ASSERT_EQ(region.available(), available);

    ASSERT_THAT(region.Read<T>(), StatusIs(StatusCode::kResourceExhausted));
    ASSERT_EQ(region.available(), available);
  }
}

TEST_F(EepromRegionTest, CreateAndCopyWriter) {
  {
    EepromRegion region(eeprom_, 0, eeprom_.length());
    EXPECT_EQ(region.start_address(), 0);
    EXPECT_EQ(region.length(), eeprom_.length());
    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.available(), eeprom_.length());

    auto copy = region;

    EXPECT_EQ(copy.start_address(), 0);
    EXPECT_EQ(copy.length(), eeprom_.length());
    EXPECT_EQ(copy.cursor(), 0);
    EXPECT_EQ(copy.available(), eeprom_.length());

    EXPECT_TRUE(region.set_cursor(1));

    EXPECT_EQ(region.start_address(), 0);
    EXPECT_EQ(region.cursor(), 1);
    EXPECT_EQ(copy.start_address(), 0);
    EXPECT_EQ(copy.cursor(), 0);

    region = copy;
  }
  {
    EepromRegion region(eeprom_, 0, eeprom_.length());
    EXPECT_EQ(region.start_address(), 0);
    EXPECT_EQ(region.length(), eeprom_.length());
    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.available(), eeprom_.length());
  }
  {
    EepromRegion region(eeprom_, eeprom_.length() - 1, 1);
    EXPECT_EQ(region.start_address(), eeprom_.length() - 1);
    EXPECT_EQ(region.length(), 1);
    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.available(), 1);

    EXPECT_TRUE(region.set_cursor(1));
    EXPECT_EQ(region.cursor(), 1);

    EXPECT_FALSE(region.set_cursor(2));
    EXPECT_EQ(region.cursor(), 1);
  }
}

TEST_F(EepromRegionTest, SetCursor) {
  const EepromAddrT kStartAddr = 10;
  const EepromAddrT kLength = 10;

  EepromRegion region(eeprom_, kStartAddr, kLength);

  EXPECT_EQ(region.cursor(), 0);

  for (EepromAddrT c = 0; c <= kLength; ++c) {
    EXPECT_TRUE(region.set_cursor(c));
    EXPECT_EQ(region.cursor(), c);
  }
  for (EepromAddrT c = kLength + 1; c < kLength * 2; ++c) {
    EXPECT_FALSE(region.set_cursor(c));
    EXPECT_EQ(region.cursor(), kLength);
  }

  for (EepromAddrT c = kLength + 1; c > 0;) {
    c--;
    EXPECT_TRUE(region.set_cursor(c));
    EXPECT_EQ(region.cursor(), c);
  }
}

TEST_F(EepromRegionTest, WriteToTinyRegion) {
  const EepromAddrT kStartAddr = 10;

  EepromRegion region(eeprom_, kStartAddr, 1);
  EXPECT_EQ(region.cursor(), 0);
  EXPECT_EQ(region.length(), 1);

  EXPECT_EQ(eeprom_[kStartAddr - 1], 0);
  EXPECT_EQ(eeprom_[kStartAddr], 0);
  EXPECT_EQ(eeprom_[kStartAddr + 1], 0);

  EXPECT_TRUE(region.Write(static_cast<uint8_t>(7)));
  EXPECT_EQ(region.cursor(), 1);
  EXPECT_FALSE(region.Write(static_cast<uint8_t>(8)));
  EXPECT_EQ(region.cursor(), 1);

  EXPECT_EQ(eeprom_[kStartAddr - 1], 0);
  EXPECT_EQ(eeprom_[kStartAddr], 7);
  EXPECT_EQ(eeprom_[kStartAddr + 1], 0);

  EXPECT_TRUE(region.set_cursor(0));
  EXPECT_EQ(region.cursor(), 0);

  EXPECT_TRUE(region.Write(static_cast<uint8_t>(8)));
  EXPECT_EQ(region.cursor(), 1);

  EXPECT_EQ(eeprom_[kStartAddr - 1], 0);
  EXPECT_EQ(eeprom_[kStartAddr], 8);
  EXPECT_EQ(eeprom_[kStartAddr + 1], 0);
}

TEST_F(EepromRegionTest, WriteAndReadCharacters) {
  const EepromAddrT kStartAddr = 10;
  EepromRegion region(eeprom_, kStartAddr, eeprom_.length() - kStartAddr);

  EXPECT_TRUE(region.Write<char>('a'));
  EXPECT_TRUE(region.Write<signed char>('b'));
  EXPECT_TRUE(region.Write<unsigned char>('c'));

  EXPECT_EQ(region.cursor(), 3);
  region.set_cursor(0);

  EXPECT_THAT(region.Read<char>(), IsOkAndHolds('a'));
  EXPECT_THAT(region.Read<signed char>(), IsOkAndHolds('b'));
  EXPECT_THAT(region.Read<unsigned char>(), IsOkAndHolds('c'));
}

TEST_F(EepromRegionTest, WriteAndReadBools) {
  const EepromAddrT kStartAddr = 10;
  EepromRegion region(eeprom_, kStartAddr, eeprom_.length() - kStartAddr);

  EXPECT_TRUE(region.Write(true));
  EXPECT_TRUE(region.Write(false));
  EXPECT_TRUE(region.Write(true));
  EXPECT_TRUE(region.Write(false));

  EXPECT_EQ(region.cursor(), 4);

  region.set_cursor(0);

  EXPECT_THAT(region.Read<bool>(), IsOkAndHolds(true));
  EXPECT_THAT(region.Read<bool>(), IsOkAndHolds(false));
  EXPECT_THAT(region.Read<bool>(), IsOkAndHolds(true));
  EXPECT_THAT(region.Read<bool>(), IsOkAndHolds(false));

  EXPECT_EQ(region.cursor(), 4);
}

TEST_F(EepromRegionTest, WriteAndReadIntegers) {
  const EepromAddrT kStartAddr = 10;

  const int8_t a = std::numeric_limits<int8_t>::min() + 1;
  const uint8_t b = std::numeric_limits<uint8_t>::max() - 1;

  const int16_t c = std::numeric_limits<int16_t>::min() + 1;
  const uint16_t d = std::numeric_limits<uint16_t>::max() - 1;

  const int32_t e = std::numeric_limits<int32_t>::min() + 1;
  const uint32_t f = std::numeric_limits<uint32_t>::max() - 1;

  EepromRegion region(eeprom_, kStartAddr, eeprom_.length() - kStartAddr);

  EXPECT_TRUE(region.Write(a));
  EXPECT_TRUE(region.Write(b));

  EXPECT_EQ(region.cursor(), 2);

  EXPECT_TRUE(region.Write(c));
  EXPECT_TRUE(region.Write(d));

  EXPECT_EQ(region.cursor(), 2 + 4);

  EXPECT_TRUE(region.Write(e));
  EXPECT_TRUE(region.Write(f));

  EXPECT_EQ(region.cursor(), 2 + 4 + 8);

  region.set_cursor(0);

  EXPECT_THAT(region.Read<int8_t>(), IsOkAndHolds(a));
  EXPECT_THAT(region.Read<uint8_t>(), IsOkAndHolds(b));

  EXPECT_EQ(region.cursor(), 2);

  EXPECT_THAT(region.Read<int16_t>(), IsOkAndHolds(c));
  EXPECT_THAT(region.Read<uint16_t>(), IsOkAndHolds(d));

  EXPECT_EQ(region.cursor(), 2 + 4);

  EXPECT_THAT(region.Read<int32_t>(), IsOkAndHolds(e));
  EXPECT_THAT(region.Read<uint32_t>(), IsOkAndHolds(f));

  EXPECT_EQ(region.cursor(), 2 + 4 + 8);

  region.set_cursor(0);
  EXPECT_TRUE(region.Write(a));

  region.Invalidate();
  EXPECT_FALSE(region.Write(a));
  EXPECT_THAT(region.Read<int8_t>(), StatusIs(StatusCode::kResourceExhausted));
}

TEST_F(EepromRegionTest, WriteAndReadFloatingPoint) {
  const EepromAddrT kStartAddr = 9;

  const float a = std::numeric_limits<float>::min();
  const float b = 0;
  const float c = std::numeric_limits<float>::epsilon();
  const float d = std::numeric_limits<float>::round_error();
  const float e = std::numeric_limits<float>::max();

  const double f = std::numeric_limits<double>::min();
  const double g = 0;
  const double h = std::numeric_limits<double>::epsilon();
  const double i = std::numeric_limits<double>::round_error();
  const double j = std::numeric_limits<double>::max();

  EepromRegion region(eeprom_, kStartAddr, eeprom_.length() - kStartAddr);

  EXPECT_TRUE(region.Write(a));
  EXPECT_TRUE(region.Write(b));
  EXPECT_TRUE(region.Write(c));
  EXPECT_TRUE(region.Write(d));
  EXPECT_TRUE(region.Write(e));

  EXPECT_EQ(region.cursor(), 5 * sizeof(float));

  EXPECT_TRUE(region.Write(f));
  EXPECT_TRUE(region.Write(g));
  EXPECT_TRUE(region.Write(h));
  EXPECT_TRUE(region.Write(i));
  EXPECT_TRUE(region.Write(j));

  EXPECT_EQ(region.cursor(), 5 * sizeof(float) + 5 * sizeof(double));

  region.set_cursor(0);

  EXPECT_THAT(region.Read<float>(), IsOkAndHolds(a));
  EXPECT_THAT(region.Read<float>(), IsOkAndHolds(b));
  EXPECT_THAT(region.Read<float>(), IsOkAndHolds(c));
  EXPECT_THAT(region.Read<float>(), IsOkAndHolds(d));
  EXPECT_THAT(region.Read<float>(), IsOkAndHolds(e));

  EXPECT_EQ(region.cursor(), 5 * sizeof(float));

  EXPECT_THAT(region.Read<double>(), IsOkAndHolds(f));
  EXPECT_THAT(region.Read<double>(), IsOkAndHolds(g));
  EXPECT_THAT(region.Read<double>(), IsOkAndHolds(h));
  EXPECT_THAT(region.Read<double>(), IsOkAndHolds(i));
  EXPECT_THAT(region.Read<double>(), IsOkAndHolds(j));

  EXPECT_EQ(region.cursor(), 5 * sizeof(float) + 5 * sizeof(double));
}

TEST_F(EepromRegionTest, WriteAndReadBytesImplicitSize) {
  const EepromAddrT kStartAddr = 11;
  const uint8_t kData[] = {9, 7, 5, 3, 1, 0, 2, 4, 6, 8};
  constexpr EepromAddrT kLength = 10;
  EXPECT_EQ(sizeof kData, kLength);

  EepromRegion region(eeprom_, kStartAddr, eeprom_.length() - kStartAddr);

  EXPECT_TRUE(region.WriteBytes(kData));

  EXPECT_EQ(region.cursor(), sizeof kData);
  region.set_cursor(0);

  uint8_t buffer[kLength];
  EXPECT_TRUE(region.ReadBytes(buffer));

  EXPECT_EQ(region.cursor(), sizeof kData);
  EXPECT_EQ(std::memcmp(kData, buffer, kLength), 0);
}

TEST_F(EepromRegionTest, WriteAndReadBytesExplicitSize) {
  const EepromAddrT kStartAddr = 11;
  const uint8_t kData[] = {9, 7, 5, 3, 1, 0, 2, 4, 6, 8};
  constexpr EepromAddrT kLength = 10;
  EXPECT_EQ(sizeof kData, kLength);

  EepromRegion region(eeprom_, kStartAddr, eeprom_.length() - kStartAddr);

  EXPECT_TRUE(region.WriteBytes(kData, sizeof kData));

  EXPECT_EQ(region.cursor(), sizeof kData);
  region.set_cursor(0);

  uint8_t buffer[kLength];
  EXPECT_TRUE(region.ReadBytes(buffer, kLength));

  EXPECT_EQ(region.cursor(), sizeof kData);
  EXPECT_EQ(std::memcmp(kData, buffer, kLength), 0);
}

TEST_F(EepromRegionTest, WriteAndReadString) {
  constexpr char kString[] = "My oh my!";
  StringView view(kString);

  const EepromAddrT kStartAddr = 23;
  EepromRegion region(eeprom_, kStartAddr, eeprom_.length() - kStartAddr);

  EXPECT_TRUE(region.WriteString(view));
  EXPECT_EQ(region.cursor(), view.size());
  region.set_cursor(0);

  char buffer[100];
  EXPECT_TRUE(region.ReadString(buffer, view.size()));
  EXPECT_EQ(region.cursor(), view.size());
  EXPECT_EQ(strncmp(buffer, kString, view.size()), 0);
}

TEST_F(EepromRegionTest, SpaceUnavailable) {
  const EepromAddrT kStartAddr = 10;
  // Big enough to store the larger of double and uint64_t.
  constexpr EepromAddrT kLength = std::max(sizeof(double), sizeof(uint64_t));
  EepromRegion region(eeprom_, kStartAddr, kLength);

  VerifyNoWriteNoRead<char>(region, 'C');
  VerifyNoWriteNoRead<unsigned char>(region, 'U');
  VerifyNoWriteNoRead<signed char>(region, 'S');
  VerifyNoWriteNoRead<int8_t>(region, 0);
  VerifyNoWriteNoRead<uint8_t>(region, 0);
  VerifyNoWriteNoRead<int16_t>(region, 0);
  VerifyNoWriteNoRead<uint16_t>(region, 0);
  VerifyNoWriteNoRead<int32_t>(region, 0);
  VerifyNoWriteNoRead<uint32_t>(region, 0);
  VerifyNoWriteNoRead<float>(region, 0);
  VerifyNoWriteNoRead<double>(region, 0);

  const uint8_t kBytes[] = {1, 2, 3};
  region.set_cursor(kLength - 2);
  EXPECT_EQ(region.available(), 2);
  EXPECT_FALSE(region.WriteBytes(kBytes, 3));
  EXPECT_EQ(region.available(), 2);

  uint8_t buffer[3];
  EXPECT_FALSE(region.ReadBytes(buffer, 3));
  EXPECT_EQ(region.available(), 2);
}

#ifdef MCU_ENABLE_DCHECK
// These tests only apply if MCU_DCHECK is enabled.
using EepromRegionDeathTest = EepromRegionTest;

TEST_F(EepromRegionDeathTest, StartsBeyondEEPROM) {
  EXPECT_DEATH_IF_SUPPORTED({ EepromRegion region(eeprom_, 65535, 2); },
                            "Starts beyond EEPROM");
}

TEST_F(EepromRegionDeathTest, ExtendsBeyondEEPROM) {
  EXPECT_DEATH_IF_SUPPORTED(
      { EepromRegion region(eeprom_, eeprom_.length() - 1, 2); },
      "Extends beyond EEPROM");
}
#endif

}  // namespace
}  // namespace test
}  // namespace mcucore
