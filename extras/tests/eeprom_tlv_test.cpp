#include "eeprom_tlv.h"

#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "gtest/gtest.h"

// 2 domains, with a declaration before the definition.
MCU_DECLARE_DOMAIN(1);
MCU_DEFINE_DOMAIN(1);
MCU_DECLARE_NAMED_DOMAIN(TestDomain2, 2);
MCU_DEFINE_NAMED_DOMAIN(TestDomain2, 2);

// 2 domains, without a declaration before the definition.
MCU_DEFINE_DOMAIN(3);
MCU_DEFINE_NAMED_DOMAIN(TestDomain4, 4);

namespace mcucore {
namespace test {
namespace {

TEST(EepromTlvTest, GetIfValidFails) {
  EEPROMClass eeprom;
  auto status_or_eeprom_tlv = EepromTlv::GetIfValid(eeprom);
  EXPECT_FALSE(status_or_eeprom_tlv.ok());
  EXPECT_EQ(status_or_eeprom_tlv.status().code(),
            StatusCode::kFailedPrecondition);
}

TEST(EepromTlvTest, ClearAndInitializeEeprom) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  auto status_or_eeprom_tlv = EepromTlv::GetIfValid(eeprom);
  ASSERT_OK(status_or_eeprom_tlv.status());
  auto eeprom_tlv = status_or_eeprom_tlv.value();
}

EepromTlv MakeValid(EEPROMClass& eeprom) {
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  auto status_or_eeprom_tlv = EepromTlv::GetIfValid(eeprom);
  CHECK(status_or_eeprom_tlv.ok()) << status_or_eeprom_tlv.status();
  ASSERT_OK(status_or_eeprom_tlv.status());
  auto eeprom_tlv = status_or_eeprom_tlv.value();
  return eeprom_tlv;
}

// Tags and the addresses to which they've been written.
using TagAddresses = std::vector<std::pair<EepromTag, EepromAddrT>>;

Status WriteDoubleFn(EepromRegion& region, double value,
                     EepromAddrT& start_address) {
  start_address = region.start_address();
  if (region.Write(value)) {
    return OkStatus();
  } else {
    return ResourceExhaustedError();
  }
}

// Base class for tests where the EEPROM contains valid contents
// at the start.
class EepromTlvValidTest : public testing::Test {
 protected:
  EepromTlvValidTest() : eeprom_tlv_(MakeValid(eeprom_)) {}

  // Using double's here as a convenient way to express unique values.
  Status WriteDoubleEntryToCursor(EepromDomain domain, uint8_t id,
                                  double value) {
    EepromTag tag{domain, id};
    EepromAddrT start_address;
    auto status = eeprom_tlv_.WriteEntryToCursor(
        tag, sizeof value, WriteDoubleFn, value, start_address);
    if (status.ok()) {
      tag_addresses.push_back(std::make_pair(tag, start_address));
    }
    return status;
  }

  StatusOr<double> ReadDouble(EepromDomain domain, uint8_t id) {}

  TagAddresses tag_addresses_;

  EEPROMClass eeprom_;
  EepromTlv eeprom_tlv_;
};

TEST_F(EepromTlvValidTest, FindEntryNotFound) {
  for (const auto domain : {MCU_DOMAIN(1), MCU_DOMAIN(TestDomain2),
                            MCU_DOMAIN(3), MCU_DOMAIN(TestDomain4)}) {
    for (const int id : 256) {
      const auto status_or_region = 
    }
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
