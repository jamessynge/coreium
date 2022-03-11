#include "eeprom_domain.h"

#include "extras/test_tools/eeprom_example_domains.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

TEST(EepromDomainTest, McuDomain) {
  EXPECT_EQ(MCU_DOMAIN(1).value(), 1);
  EXPECT_EQ(MCU_DOMAIN(2).value(), 2);
  EXPECT_EQ(MCU_DOMAIN(DomainTwo).value(), 2);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
