#include "eeprom_tag.h"

#include "extras/test_tools/eeprom_example_domains.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

using ::mcucore::PrintValueToStdString;

TEST(EepromTagTest, UseMcuDomain) {
  EXPECT_EQ(MCU_DOMAIN(1).value(), 1);
  EXPECT_EQ(MCU_DOMAIN(2).value(), 2);
  EXPECT_EQ(MCU_DOMAIN(DomainTwo).value(), 2);
}

TEST(EepromTagTest, IsReservedDomain) {
  EXPECT_TRUE(IsReservedDomain(internal::MakeEepromDomain(0)));
  EXPECT_TRUE(IsReservedDomain(internal::MakeEepromDomain(255)));

  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(1)));
  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(2)));
  EXPECT_FALSE(IsReservedDomain(MCU_DOMAIN(DomainTwo)));
}

TEST(EepromTagTest, InsertInto) {
  EXPECT_EQ(PrintValueToStdString(EepromTag{MCU_DOMAIN(1), 3}),
            "{.domain=1, .id=3}");
}

TEST(EepromTagTest, Compare) {
  const auto tag1 = EepromTag{MCU_DOMAIN(1), 3};
  const auto tag2 = EepromTag{MCU_DOMAIN(DomainTwo), 255};
  EXPECT_EQ(tag1, tag1);
  EXPECT_EQ(tag2, tag2);
  EXPECT_EQ(tag2, (EepromTag{MCU_DOMAIN(2), 255}));

  EXPECT_NE(tag1, tag2);
  EXPECT_NE(tag2, tag1);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
