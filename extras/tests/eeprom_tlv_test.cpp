#include "eeprom_tlv.h"

#include "eeprom_region.h"
#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"
#include "string_view.h"

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

TEST(EepromTlvGetIfValidTest, FailsWithNoPrefix) {
  EEPROMClass eeprom;
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kNotFound));
}

TEST(EepromTlvGetIfValidTest, FailsWithZeroBeyondAddr) {
  EEPROMClass eeprom;
  EepromRegion(eeprom).WriteString(StringView("Tlv!"));
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kDataLoss));
}

TEST(EepromTlvGetIfValidTest, FailsWithTooLargeBeyondAddr) {
  EEPROMClass eeprom;
  for (EepromAddrT addr = 0; addr < 100; ++addr) {
    eeprom.write(addr, 0xFF);
  }
  EepromRegion(eeprom).WriteString(StringView("Tlv!"));
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kDataLoss));
}

TEST(EepromTlvGetIfValidTest, FailsWithWrongCrc) {
  EEPROMClass eeprom;
  for (EepromAddrT addr = 0; addr < 100; ++addr) {
    eeprom.write(addr, 0x01);
  }
  EepromRegion(eeprom).WriteString(StringView("Tlv!"));
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kDataLoss));
}

TEST(EepromTlvGetIfValidTest, ClearAndInitializeEeprom) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  auto status_or_eeprom_tlv = EepromTlv::GetIfValid(eeprom);
  ASSERT_STATUS_OK(status_or_eeprom_tlv.status());
}

StatusOr<EepromTlv> MakeEmpty(EEPROMClass& eeprom) {
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  return EepromTlv::GetIfValid(eeprom);
}

EepromTlv MakeEmptyOrDie(EEPROMClass& eeprom) {
  auto status_or_eeprom_tlv = MakeEmpty(eeprom);
  MCU_CHECK_OK(status_or_eeprom_tlv.status());
  return status_or_eeprom_tlv.value();
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

}  // namespace

// Base class for tests where the EEPROM contains valid contents at the start.
// Test is NOT in anonymous namespace so that we can declare it a friend of
// EepromTlv.
class EepromTlvTest : public testing::Test {
 protected:
  EepromTlvTest() : eeprom_tlv_(MakeEmptyOrDie(eeprom_)) {}

  auto Available() { return eeprom_tlv_.Available(); }

  // Using double's here as a convenient way to express unique values.
  Status WriteDoubleEntryToCursor(EepromDomain domain, uint8_t id,
                                  double value) {
    EepromTag tag{domain, id};
    EepromAddrT start_address;
    auto status = eeprom_tlv_.WriteEntryToCursor(
        tag, sizeof value, WriteDoubleFn, value, start_address);
    if (status.ok()) {
      tag_addresses_.push_back(std::make_pair(tag, start_address));
    }
    return status;
  }

  // StatusOr<double> ReadDouble(EepromDomain domain, uint8_t id) {}

  TagAddresses tag_addresses_;

  EEPROMClass eeprom_;
  EepromTlv eeprom_tlv_;
};

namespace {

TEST_F(EepromTlvTest, LotsAvailable) {
  EXPECT_EQ(Available(), eeprom_.length() - EepromTlv::kFixedHeaderSize -
                             EepromTlv::kEntryHeaderSize);
}

TEST_F(EepromTlvTest, FindEntryNotFound) {
  for (const auto domain : {MCU_DOMAIN(1), MCU_DOMAIN(TestDomain2),
                            MCU_DOMAIN(3), MCU_DOMAIN(TestDomain4)}) {
    for (int id = 0; id <= 255; ++id) {
      EepromTag tag{domain, static_cast<uint8_t>(id)};
      EXPECT_THAT(eeprom_tlv_.FindEntry(tag), StatusIs(StatusCode::kNotFound));
    }
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
