#include "eeprom_tlv.h"

#include "extras/host/eeprom/eeprom.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

TEST(EepromTlvTest, SomeTest) {
  EEPROMClass eeprom;
  auto status_or_eeprom_tlv = EepromTlv::GetIfValid(eeprom);
  EXPECT_FALSE(status_or_eeprom_tlv.ok());
  EXPECT_EQ(status_or_eeprom_tlv.status().code(),
            StatusCode::kFailedPrecondition);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
