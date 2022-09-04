#include "container/flash_string_table.h"

#include "gtest/gtest.h"
#include "strings/progmem_string_data.h"

namespace mcucore {
namespace test {
namespace {

MCU_FLASH_STRING_TABLE(table, MCU_PSD("Entry0"), MCU_PSD("Entry1"),
                       MCU_PSD("Entry2"));

// Size is implicit (determined via inferred non-type template parameter).
TEST(FlashStringTableTest, LookupFlashString_ImplicitSize_Int) {
  EXPECT_EQ(LookupFlashString(table, 0), MCU_FLASHSTR("Entry0"));
  EXPECT_EQ(LookupFlashString(table, 1), MCU_FLASHSTR("Entry1"));
  EXPECT_EQ(LookupFlashString(table, 2), MCU_FLASHSTR("Entry2"));
  EXPECT_EQ(LookupFlashString(table, 3), nullptr);
}

// Table size is passed explicitly.
TEST(FlashStringTableTest, LookupFlashString_ExplicitSize_Int) {
  EXPECT_EQ(LookupFlashString(table, 3, 0), MCU_FLASHSTR("Entry0"));
  EXPECT_EQ(LookupFlashString(table, 3, 1), MCU_FLASHSTR("Entry1"));
  EXPECT_EQ(LookupFlashString(table, 3, 2), MCU_FLASHSTR("Entry2"));
  EXPECT_EQ(LookupFlashString(table, 3, 3), nullptr);
}

// Zero-based enum.
enum EnumZero { kZero, kOne, kTwo };
constexpr auto kBogusThree = static_cast<EnumZero>(3);

TEST(FlashStringTableTest, LookupFlashStringForDenseEnum_MinZero) {
  EXPECT_EQ(LookupFlashStringForDenseEnum(table, kZero),
            MCU_FLASHSTR("Entry0"));
  EXPECT_EQ(LookupFlashStringForDenseEnum(table, kOne), MCU_FLASHSTR("Entry1"));
  EXPECT_EQ(LookupFlashStringForDenseEnum(table, kTwo), MCU_FLASHSTR("Entry2"));
  EXPECT_EQ(LookupFlashStringForDenseEnum(table, kBogusThree), nullptr);
}

// First value above zero.
enum EnumFive { kFive = 5, kSix, kSeven };
constexpr auto kBogusFour = static_cast<EnumFive>(4);
constexpr auto kBogusEight = static_cast<EnumFive>(8);

TEST(FlashStringTableTest, LookupFlashStringForDenseEnum_MinGreaterThanZero) {
  EXPECT_EQ(LookupFlashStringForDenseEnum<int>(table, kFive, kSeven, kFive),
            MCU_FLASHSTR("Entry0"));
  EXPECT_EQ(LookupFlashStringForDenseEnum<int>(table, kFive, kSeven, kSix),
            MCU_FLASHSTR("Entry1"));
  EXPECT_EQ(LookupFlashStringForDenseEnum<int>(table, kFive, kSeven, kSeven),
            MCU_FLASHSTR("Entry2"));

  EXPECT_EQ(
      LookupFlashStringForDenseEnum<int>(table, kFive, kSeven, kBogusFour),
      nullptr);
  EXPECT_EQ(
      LookupFlashStringForDenseEnum<int>(table, kFive, kSeven, kBogusEight),
      nullptr);
}

// First value below zero.
enum class EnumNeg { kB = -1, kC, kD };
constexpr auto kBogusA = static_cast<EnumNeg>(-2);
constexpr auto kBogusE = static_cast<EnumNeg>(2);

TEST(FlashStringTableTest, LookupFlashStringForDenseEnum_MinLessThanZero) {
  EXPECT_EQ(LookupFlashStringForDenseEnum<int>(table, EnumNeg::kB, EnumNeg::kD,
                                               EnumNeg::kB),
            MCU_FLASHSTR("Entry0"));
  EXPECT_EQ(LookupFlashStringForDenseEnum<int>(table, EnumNeg::kB, EnumNeg::kD,
                                               EnumNeg::kC),
            MCU_FLASHSTR("Entry1"));
  EXPECT_EQ(LookupFlashStringForDenseEnum<int>(table, EnumNeg::kB, EnumNeg::kD,
                                               EnumNeg::kD),
            MCU_FLASHSTR("Entry2"));

  EXPECT_EQ(LookupFlashStringForDenseEnum<int>(table, EnumNeg::kB, EnumNeg::kD,
                                               kBogusA),
            nullptr);
  EXPECT_EQ(LookupFlashStringForDenseEnum<int>(table, EnumNeg::kB, EnumNeg::kD,
                                               kBogusE),
            nullptr);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
