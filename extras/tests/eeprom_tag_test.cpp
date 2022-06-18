#include "eeprom_tag.h"

#include "extras/test_tools/eeprom_example_domains.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "gtest/gtest.h"

MCU_DEFINE_CONSTEXPR_NAMED_DOMAIN(NinetyEight, 98);
MCU_DEFINE_CONSTEXPR_DOMAIN(99);

namespace mcucore {
namespace test {
namespace {

using ::mcucore::PrintValueToStdString;

TEST(EepromTagTest, UseMcuDomain) {
  EXPECT_EQ(MCU_DOMAIN(1).value(), 1);
  EXPECT_EQ(MCU_DOMAIN(2).value(), 2);
  EXPECT_EQ(MCU_DOMAIN(DomainTwo).value(), 2);

  EXPECT_EQ(MCU_DOMAIN(NinetyEight).value(), 98);
  EXPECT_EQ(MCU_DOMAIN(98).value(), 98);
  EXPECT_EQ(MCU_DOMAIN(99).value(), 99);
}

TEST(EepromTagTest, IsReservedDomain) {
  EXPECT_TRUE(IsReservedDomain(internal::MakeEepromDomain(0)));
  EXPECT_TRUE(IsReservedDomain(internal::MakeEepromDomain(255)));

  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(1)));
  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(2)));
  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(DomainTwo)));

  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(NinetyEight)));
  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(98)));
  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(99)));
}

TEST(EepromTagTest, InsertInto) {
  EXPECT_EQ(PrintValueToStdString(EepromTag{MCU_DOMAIN(1), 3}),
            "{.domain=1, .id=3}");
  EXPECT_EQ(PrintValueToStdString(EepromTag{MCU_DOMAIN(NinetyEight), 5}),
            "{.domain=98, .id=5}");
}

TEST(EepromTagTest, Compare) {
  const auto tag1 = EepromTag{MCU_DOMAIN(1), 3};
  const auto tag2 = EepromTag{MCU_DOMAIN(DomainTwo), 255};

  EXPECT_EQ(tag1.domain.value(), 1);
  EXPECT_EQ(tag1.id, 3);
  EXPECT_EQ(tag1, tag1);
  EXPECT_EQ(tag1, (EepromTag{MCU_DOMAIN(1), 3}));

  EXPECT_EQ(tag2.domain.value(), 2);
  EXPECT_EQ(tag2.id, 255);
  EXPECT_EQ(tag2, tag2);
  EXPECT_EQ(tag2, (EepromTag{MCU_DOMAIN(2), 255}));

  EXPECT_NE(tag1, tag2);
  EXPECT_NE(tag2, tag1);
}

TEST(EepromTagTest, CompareConstexpr) {
  constexpr auto tag1 = EepromTag{MCU_DOMAIN(NinetyEight), 100};
  constexpr auto tag2 = EepromTag{MCU_DOMAIN(99), 97};

  EXPECT_EQ(tag1, tag1);
  EXPECT_EQ(tag2, tag2);

  EXPECT_EQ(tag1, (EepromTag{MCU_DOMAIN(98), 100}));
  EXPECT_EQ(tag2, (EepromTag{MCU_DOMAIN(99), 97}));

  EXPECT_NE(tag1, tag2);
  EXPECT_NE(tag2, tag1);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
