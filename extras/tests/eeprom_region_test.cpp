#include "eeprom_region.h"

#include <algorithm>
#include <cstring>
#include <limits>

#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "gtest/gtest.h"
#include "string_view.h"

namespace mcucore {
namespace test {
namespace {

class EepromRegionTest : public testing::Test {
 public:
  using AddrT = EepromRegion::AddrT;
  using SizeT = EepromRegion::SizeT;
  using CursorT = EepromRegion::SizeT;

  template <typename T>
  void VerifyNoWriteNoRead(EepromRegion& region, const T value) {
    for (SizeT available = 0; available < sizeof(T); ++available) {
      ASSERT_GE(region.size(), available);
      ASSERT_TRUE(region.set_cursor(region.size() - available));
      ASSERT_EQ(region.available(), available);

      ASSERT_FALSE(region.Write<T>(value));
      ASSERT_EQ(region.available(), available);

      T t;
      ASSERT_FALSE(region.ReadInto(t));
      ASSERT_EQ(region.available(), available);

      ASSERT_EQ(region.Read<T>(), Status(EepromRegion::kResourceExhausted));
      ASSERT_EQ(region.available(), available);
    }
  }

 protected:
  void SetUp() override {  // Reset the EEPROM to all zeroes.
    EEPROM = EEPROMClass();
  }
};

TEST_F(EepromRegionTest, OkSize) {
  {
    EepromRegion region(0, 1);
    EXPECT_EQ(region.start_address(), 0);
    EXPECT_EQ(region.size(), 1);
    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.available(), 1);

    EXPECT_TRUE(region.set_cursor(1));
    EXPECT_EQ(region.cursor(), 1);

    EXPECT_FALSE(region.set_cursor(2));
    EXPECT_EQ(region.cursor(), 1);
  }
  {
    EepromRegion region(0, EepromRegion::kMaxAddrT);
    EXPECT_EQ(region.start_address(), 0);
    EXPECT_EQ(region.size(), EepromRegion::kMaxAddrT);
    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.available(), EepromRegion::kMaxAddrT);
  }
  {
    EepromRegion region(EepromRegion::kMaxAddrT, 1);
    EXPECT_EQ(region.start_address(), EepromRegion::kMaxAddrT);
    EXPECT_EQ(region.size(), 1);
    EXPECT_EQ(region.cursor(), 0);
    EXPECT_EQ(region.available(), 1);

    EXPECT_TRUE(region.set_cursor(1));
    EXPECT_EQ(region.cursor(), 1);

    EXPECT_FALSE(region.set_cursor(2));
    EXPECT_EQ(region.cursor(), 1);
  }
}

TEST_F(EepromRegionTest, SetCursor) {
  const EepromRegion::AddrT kStartAddr = 10;
  const EepromRegion::SizeT kSize = 10;

  EepromRegion region(kStartAddr, kSize);

  EXPECT_EQ(region.cursor(), 0);

  for (CursorT c = 0; c <= kSize; ++c) {
    EXPECT_TRUE(region.set_cursor(c));
    EXPECT_EQ(region.cursor(), c);
  }
  for (CursorT c = kSize + 1; c < kSize * 2; ++c) {
    EXPECT_FALSE(region.set_cursor(c));
    EXPECT_EQ(region.cursor(), kSize);
  }

  for (CursorT c = kSize + 1; c > 0;) {
    c--;
    EXPECT_TRUE(region.set_cursor(c));
    EXPECT_EQ(region.cursor(), c);
  }
}

TEST_F(EepromRegionTest, WriteToTinyRegion) {
  const EepromRegion::AddrT kStartAddr = 10;

  EepromRegion region(kStartAddr, 1);
  EXPECT_EQ(region.cursor(), 0);
  EXPECT_EQ(region.size(), 1);

  EXPECT_EQ(EEPROM[kStartAddr - 1], 0);
  EXPECT_EQ(EEPROM[kStartAddr], 0);
  EXPECT_EQ(EEPROM[kStartAddr + 1], 0);

  EXPECT_TRUE(region.Write(static_cast<uint8_t>(7)));
  EXPECT_EQ(region.cursor(), 1);
  EXPECT_FALSE(region.Write(static_cast<uint8_t>(8)));
  EXPECT_EQ(region.cursor(), 1);

  EXPECT_EQ(EEPROM[kStartAddr - 1], 0);
  EXPECT_EQ(EEPROM[kStartAddr], 7);
  EXPECT_EQ(EEPROM[kStartAddr + 1], 0);

  EXPECT_TRUE(region.set_cursor(0));
  EXPECT_EQ(region.cursor(), 0);

  EXPECT_TRUE(region.Write(static_cast<uint8_t>(8)));
  EXPECT_EQ(region.cursor(), 1);

  EXPECT_EQ(EEPROM[kStartAddr - 1], 0);
  EXPECT_EQ(EEPROM[kStartAddr], 8);
  EXPECT_EQ(EEPROM[kStartAddr + 1], 0);
}

TEST_F(EepromRegionTest, WriteAndReadCharacters) {
  const EepromRegion::AddrT kStartAddr = 10;
  EepromRegion region(kStartAddr);

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
  const EepromRegion::AddrT kStartAddr = 10;
  EepromRegion region(kStartAddr);

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
  const EepromRegion::AddrT kStartAddr = 10;

  const int8_t a = std::numeric_limits<int8_t>::min() + 1;
  const uint8_t b = std::numeric_limits<uint8_t>::max() - 1;

  const int16_t c = std::numeric_limits<int16_t>::min() + 1;
  const uint16_t d = std::numeric_limits<uint16_t>::max() - 1;

  const int32_t e = std::numeric_limits<int32_t>::min() + 1;
  const uint32_t f = std::numeric_limits<uint32_t>::max() - 1;

  EepromRegion region(kStartAddr);

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
}

TEST_F(EepromRegionTest, WriteAndReadFloatingPoint) {
  const EepromRegion::AddrT kStartAddr = 9;

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

  EepromRegion region(kStartAddr);

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
  const EepromRegion::AddrT kStartAddr = 11;
  const uint8_t kData[] = {9, 7, 5, 3, 1, 0, 2, 4, 6, 8};
  constexpr SizeT kSize = 10;
  EXPECT_EQ(sizeof kData, kSize);

  EepromRegion region(kStartAddr);

  EXPECT_TRUE(region.WriteBytes(kData, sizeof kData));

  EXPECT_EQ(region.cursor(), sizeof kData);
  region.set_cursor(0);

  uint8_t buffer[kSize];
  EXPECT_TRUE(region.ReadBytes(buffer, kSize));

  EXPECT_EQ(region.cursor(), sizeof kData);
  EXPECT_EQ(std::memcmp(kData, buffer, kSize), 0);
}

TEST_F(EepromRegionTest, WriteAndReadString) {
  constexpr char kString[] = "My oh my!";
  StringView view(kString);

  const EepromRegion::AddrT kStartAddr = 23;
  EepromRegion region(kStartAddr);

  EXPECT_TRUE(region.WriteString(view));
  EXPECT_EQ(region.cursor(), view.size());
  region.set_cursor(0);

  char buffer[100];
  EXPECT_TRUE(region.ReadString(buffer, view.size()));
  EXPECT_EQ(region.cursor(), view.size());
  EXPECT_EQ(strncmp(buffer, kString, view.size()), 0);
}

TEST_F(EepromRegionTest, SpaceUnavailable) {
  const EepromRegion::AddrT kStartAddr = 10;
  constexpr SizeT kSize = std::max(sizeof(double), sizeof(uint64_t));
  EepromRegion region(kStartAddr, kSize);

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
  region.set_cursor(kSize - 2);
  EXPECT_EQ(region.available(), 2);
  EXPECT_FALSE(region.WriteBytes(kBytes, 3));
  EXPECT_EQ(region.available(), 2);

  uint8_t buffer[3];
  EXPECT_FALSE(region.ReadBytes(buffer, 3));
  EXPECT_EQ(region.available(), 2);
}

using EepromRegionDeathTest = EepromRegionTest;

TEST_F(EepromRegionDeathTest, Size0) {
  EXPECT_DEATH_IF_SUPPORTED({ EepromRegion region(1, 0); }, "0 < size");
}

TEST_F(EepromRegionDeathTest, BeyondAddressableRange) {
  EXPECT_DEATH_IF_SUPPORTED({ EepromRegion region(65535, 2); },
                            "Overflows region");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
