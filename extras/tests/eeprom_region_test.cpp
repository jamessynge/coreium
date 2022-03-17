#include "eeprom_region.h"

#include <algorithm>
#include <cstring>
#include <limits>

#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "gtest/gtest.h"
#include "status_code.h"
#include "string_view.h"

namespace mcucore {
namespace test {
namespace {

class EepromRegionReaderTest : public testing::Test {
 protected:
  EEPROMClass eeprom_;
};

TEST_F(EepromRegionReaderTest, CreateAndCopy) {
  auto validator = [](EepromRegionReader& reader,
                      const EepromAddrT start_address,
                      const EepromAddrT region_length) {
    EXPECT_EQ(reader.start_address(), start_address);
    EXPECT_EQ(reader.length(), region_length);
    EXPECT_EQ(reader.cursor(), 0);
    EXPECT_EQ(reader.available(), region_length);

    auto copy = reader;

    EXPECT_EQ(copy.start_address(), start_address);
    EXPECT_EQ(copy.length(), region_length);
    EXPECT_EQ(copy.cursor(), 0);
    EXPECT_EQ(copy.available(), region_length);

    EXPECT_TRUE(reader.set_cursor(1));

    EXPECT_EQ(reader.start_address(), start_address);
    EXPECT_EQ(reader.cursor(), 1);
    EXPECT_EQ(copy.start_address(), start_address);
    EXPECT_EQ(copy.cursor(), 0);

    reader = copy;

    EXPECT_EQ(reader.start_address(), start_address);
    EXPECT_EQ(reader.cursor(), 0);
    EXPECT_EQ(copy.start_address(), start_address);
    EXPECT_EQ(copy.cursor(), 0);

    reader.Invalidate();

    EXPECT_EQ(reader.cursor(), 0);
    EXPECT_EQ(reader.length(), 0);
    EXPECT_EQ(reader.available(), 0);

    reader = copy;

    EXPECT_EQ(reader.cursor(), 0);
    EXPECT_EQ(reader.length(), region_length);
    EXPECT_EQ(reader.available(), region_length);
  };

  {
    EepromRegionReader reader(eeprom_);
    validator(reader, 0, eeprom_.length());
  }
  {
    EepromRegionReader reader(eeprom_, 10, 100);
    validator(reader, 10, 100);
  }
}

class EepromRegionTest : public EepromRegionReaderTest {
 protected:
};

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

    ASSERT_EQ(region.Read<T>(), Status(StatusCode::kResourceExhausted));
    ASSERT_EQ(region.available(), available);
  }
}

TEST_F(EepromRegionTest, CreateAndCopyWriter) {
  {
    EepromRegion region(eeprom_);
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
  EepromRegion region(eeprom_, kStartAddr);

  EXPECT_TRUE(region.Write<char>('a'));
  EXPECT_TRUE(region.Write<signed char>('b'));
  EXPECT_TRUE(region.Write<unsigned char>('c'));

  EXPECT_EQ(region.cursor(), 3);
  region.set_cursor(0);

  EXPECT_EQ(region.Read<char>(), StatusOr<char>('a'));
  EXPECT_EQ(region.Read<signed char>(), StatusOr<signed char>('b'));
  EXPECT_EQ(region.Read<unsigned char>(), StatusOr<unsigned char>('c'));
}

TEST_F(EepromRegionTest, WriteAndReadBools) {
  const EepromAddrT kStartAddr = 10;
  EepromRegion region(eeprom_, kStartAddr);

  EXPECT_TRUE(region.Write(true));
  EXPECT_TRUE(region.Write(false));
  EXPECT_TRUE(region.Write(true));
  EXPECT_TRUE(region.Write(false));

  EXPECT_EQ(region.cursor(), 4);

  region.set_cursor(0);

  EXPECT_EQ(region.Read<bool>(), true);
  EXPECT_EQ(region.Read<bool>(), false);
  EXPECT_EQ(region.Read<bool>(), true);
  EXPECT_EQ(region.Read<bool>(), false);

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

  EepromRegion region(eeprom_, kStartAddr);

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

  EXPECT_EQ(region.Read<int8_t>(), a);
  EXPECT_EQ(region.Read<uint8_t>(), b);

  EXPECT_EQ(region.cursor(), 2);

  EXPECT_EQ(region.Read<int16_t>(), c);
  EXPECT_EQ(region.Read<uint16_t>(), d);

  EXPECT_EQ(region.cursor(), 2 + 4);

  EXPECT_EQ(region.Read<int32_t>(), e);
  EXPECT_EQ(region.Read<uint32_t>(), f);

  EXPECT_EQ(region.cursor(), 2 + 4 + 8);

  region.set_cursor(0);
  EXPECT_TRUE(region.Write(a));

  region.Invalidate();
  EXPECT_FALSE(region.Write(a));
  EXPECT_EQ(region.Read<int8_t>(), Status(StatusCode::kResourceExhausted));
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

  EepromRegion region(eeprom_, kStartAddr);

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

  EXPECT_EQ(region.Read<float>(), a);
  EXPECT_EQ(region.Read<float>(), b);
  EXPECT_EQ(region.Read<float>(), c);
  EXPECT_EQ(region.Read<float>(), d);
  EXPECT_EQ(region.Read<float>(), e);

  EXPECT_EQ(region.cursor(), 5 * sizeof(float));

  EXPECT_EQ(region.Read<double>(), f);
  EXPECT_EQ(region.Read<double>(), g);
  EXPECT_EQ(region.Read<double>(), h);
  EXPECT_EQ(region.Read<double>(), i);
  EXPECT_EQ(region.Read<double>(), j);

  EXPECT_EQ(region.cursor(), 5 * sizeof(float) + 5 * sizeof(double));
}

TEST_F(EepromRegionTest, WriteAndReadBytes) {
  const EepromAddrT kStartAddr = 11;
  const uint8_t kData[] = {9, 7, 5, 3, 1, 0, 2, 4, 6, 8};
  constexpr EepromAddrT kLength = 10;
  EXPECT_EQ(sizeof kData, kLength);

  EepromRegion region(eeprom_, kStartAddr);

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
  EepromRegion region(eeprom_, kStartAddr);

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

}  // namespace
}  // namespace test
}  // namespace mcucore
