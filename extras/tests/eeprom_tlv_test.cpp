#include "eeprom_tlv.h"

#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

class EepromTlvTest : public testing::Test {
 protected:
  EepromTlvTest() : eeprom_tlv_(eeprom_) {}
  EEPROMClass eeprom_;
  EepromTlv eeprom_tlv_;
};

TEST_F(EepromTlvTest, SomeTest) {
  //
  EXPECT_EQ(1, 1);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
