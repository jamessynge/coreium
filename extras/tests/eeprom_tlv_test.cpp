#include "eeprom_tlv.h"

#include <cstdint>
#include <string>
#include <utility>

#include "absl/strings/match.h"
#include "eeprom_domain.h"
#include "eeprom_region.h"
#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"
#include "status.h"
#include "status_or.h"
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

using ::testing::HasSubstr;
using ::testing::Not;

constexpr EepromAddrT kAddressOfFirstEntryAddress = 4;
constexpr EepromAddrT kAddressOfCrc = 6;
constexpr EepromAddrT kAddressOfFirstEntry = 10;
constexpr EepromAddrT kOffsetOfEntryDataLength = 2;

TEST(EepromTlvGetIfValidTest, FailsWithNoPrefix) {
  EEPROMClass eeprom;
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kNotFound));
}

TEST(EepromTlvGetIfValidTest, FailsWithZeroBeyondAddr) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  eeprom.put(kAddressOfFirstEntryAddress, EepromAddrT(0));
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kDataLoss));
}

TEST(EepromTlvGetIfValidTest, FailsWithTooLargeBeyondAddr) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  eeprom.put(kAddressOfFirstEntryAddress, EepromAddrT(65535));
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kDataLoss));
}

TEST(EepromTlvGetIfValidTest, ClearAndInitializeEeprom) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  auto status_or_eeprom_tlv = EepromTlv::GetIfValid(eeprom);
  ASSERT_STATUS_OK(status_or_eeprom_tlv.status());
}

TEST(EepromTlvGetIfValidTest, FailsWithWrongCrc) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  // Corrupt the CRC.
  eeprom.write(kAddressOfCrc, ~eeprom.read(kAddressOfCrc));
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kDataLoss));
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

// Captures the start address of the data region in start_address. The cursor is
// not captured at the end because it is assumed (and can be checked elsewhere)
// that it is start_address + sizeof(double).
Status WriteDoubleFn(EepromRegion& region, double value,
                     EepromAddrT& start_address) {
  start_address = region.start_address();
  if (region.Write(value)) {
    return OkStatus();
  } else {
    return ResourceExhaustedError();
  }
}

// Captures the start address of the data region in start_address.
Status WriteStringFn(EepromRegion& region, StringView value,
                     EepromAddrT& start_address) {
  start_address = region.start_address();
  if (region.WriteString(value)) {
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

  // Using doubles here as a convenient way to express unique values. Returns
  // the start address of the data region if successful.
  StatusOr<EepromAddrT> WriteDoubleEntryToCursor(const EepromDomain domain,
                                                 const uint8_t id,
                                                 const double value) {
    MCU_RETURN_IF_ERROR(eeprom_tlv_.Validate());
    EepromTag tag{domain, id};
    EepromAddrT start_address;
    MCU_RETURN_IF_ERROR(eeprom_tlv_.WriteEntryToCursor(
        tag, sizeof value, WriteDoubleFn, value, start_address));
    return start_address;
  }

  StatusOr<std::pair<EepromAddrT, double>> ReadLocationAndDouble(
      const EepromDomain domain, const uint8_t id) {
    MCU_RETURN_IF_ERROR(eeprom_tlv_.Validate());
    EepromTag tag{domain, id};
    MCU_ASSIGN_OR_RETURN(auto entry_region, eeprom_tlv_.FindEntry(tag));
    MCU_ASSIGN_OR_RETURN(const auto value, entry_region.Read<double>());
    return std::make_pair(entry_region.start_address(), value);
  }

  StatusOr<double> ReadDouble(const EepromDomain domain, const uint8_t id) {
    MCU_ASSIGN_OR_RETURN(const auto read_result,
                         ReadLocationAndDouble(domain, id));
    return read_result.second;
  }

  Status WriteAndReadDouble(const EepromDomain domain, const uint8_t id,
                            const double value) {
    MCU_ASSIGN_OR_RETURN(const auto start_address_of_write,
                         WriteDoubleEntryToCursor(domain, id, value));
    MCU_ASSIGN_OR_RETURN(const auto read_result,
                         ReadLocationAndDouble(domain, id));
    EXPECT_EQ(start_address_of_write, read_result.first);
    EXPECT_EQ(read_result.second, value);
    return OkStatus();
  }

  // Using strings here as a convenient way to express values that are of
  // varying lengths.
  Status WriteStringEntryToCursor(EepromDomain domain, uint8_t id,
                                  StringView value) {
    MCU_CHECK_OK(eeprom_tlv_.Validate());
    EepromTag tag{domain, id};
    EepromAddrT start_address;

    MCU_RETURN_IF_ERROR(eeprom_tlv_.WriteEntryToCursor(
        tag, value.size(), WriteStringFn, value, start_address));
    tag_addresses_.push_back(std::make_pair(tag, start_address));
    MCU_CHECK_OK(eeprom_tlv_.Validate());
    auto status_or_entry_region = eeprom_tlv_.FindEntry(tag);
    MCU_CHECK_OK(status_or_entry_region);
    auto entry_region = status_or_entry_region.value();
    EXPECT_EQ(entry_region.start_address(), start_address);
    EXPECT_EQ(entry_region.length(), value.size());
    // TODO Add check of stored value
    return OkStatus();
  }

  auto FindEntry(EepromDomain domain, uint8_t id) {
    EepromTag tag{domain, id};
    return eeprom_tlv_.FindEntry(tag);
  }

  auto RemoveEntry(EepromDomain domain, uint8_t id) {
    EepromTag tag{domain, id};
    return eeprom_tlv_.RemoveEntry(tag);
  }

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

TEST_F(EepromTlvTest, WriteAndReadOneDouble) {
  // There should be no entries in domain 1.
  for (int id = 0; id <= 255; ++id) {
    EepromTag tag{MCU_DOMAIN(1), static_cast<uint8_t>(id)};
    EXPECT_THAT(eeprom_tlv_.FindEntry(tag), StatusIs(StatusCode::kNotFound));
  }

  EXPECT_STATUS_OK(WriteAndReadDouble(MCU_DOMAIN(1), 1, 3.3));

  // There should be no entries in domain 1, except for id 1.
  for (int id = 0; id <= 255; ++id) {
    EepromTag tag{MCU_DOMAIN(1), static_cast<uint8_t>(id)};
    if (id == 1) {
      EXPECT_STATUS_OK(eeprom_tlv_.FindEntry(tag));
    } else {
      EXPECT_THAT(eeprom_tlv_.FindEntry(tag), StatusIs(StatusCode::kNotFound));
    }
  }
}

TEST_F(EepromTlvTest, VariousDoubleEntries) {
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));

  // Write a bunch of entries.
  for (const auto domain : {MCU_DOMAIN(1), MCU_DOMAIN(TestDomain2),
                            MCU_DOMAIN(3), MCU_DOMAIN(TestDomain4)}) {
    for (const uint8_t id : {7, 21, 33}) {
      const double value = domain.value() / static_cast<double>(id);
      EXPECT_THAT(FindEntry(domain, id), StatusIs(StatusCode::kNotFound));
      EXPECT_STATUS_OK(WriteAndReadDouble(domain, id, value));

      EXPECT_THAT(PrintValueToStdString(eeprom_tlv_),
                  Not(HasSubstr("(Empty)")));
    }
  }

  // Read each entry, then replace it.
  for (const auto domain : {MCU_DOMAIN(1), MCU_DOMAIN(TestDomain2),
                            MCU_DOMAIN(3), MCU_DOMAIN(TestDomain4)}) {
    for (const uint8_t id : {7, 21, 33}) {
      const double expected_value = domain.value() / static_cast<double>(id);

      ASSERT_THAT(ReadDouble(domain, id), IsOkAndHolds(expected_value));

      const double new_value = static_cast<double>(id) / domain.value();
      EXPECT_STATUS_OK(WriteDoubleEntryToCursor(domain, id, new_value));
      PrintValueToStdString(eeprom_tlv_);
    }
  }

  // Read each replaced entry, then remove it.
  for (const auto domain : {MCU_DOMAIN(1), MCU_DOMAIN(TestDomain2),
                            MCU_DOMAIN(3), MCU_DOMAIN(TestDomain4)}) {
    for (const uint8_t id : {7, 21, 33}) {
      const double expected_value = static_cast<double>(id) / domain.value();
      ASSERT_THAT(ReadDouble(domain, id), IsOkAndHolds(expected_value));

      EepromTag tag{domain, id};
      ASSERT_STATUS_OK(eeprom_tlv_.RemoveEntry(tag));
      EXPECT_THAT(FindEntry(domain, id), StatusIs(StatusCode::kNotFound));
      PrintValueToStdString(eeprom_tlv_);
    }
  }

  // The printed debug info should include the string "(Empty)", because
  // removing the last entry should cause a reset back to the "just initialized"
  // state.
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));
}

TEST_F(EepromTlvTest, FillEeprom) {
  // Fills the EEPROM with the same string entry again and again; once full,
  // shrink the string until we're able to write some more or the string is
  // empty.
  auto fill_function = [&](StringView s) {
    while (true) {
      ASSERT_STATUS_OK(eeprom_tlv_.Validate());
      Status status = WriteStringEntryToCursor(MCU_DOMAIN(1), 1, s);
      if (IsResourceExhausted(status)) {
        if (s.empty()) {
          return;
        } else {
          s.remove_prefix(s.size());
        }
      } else {
        ASSERT_STATUS_OK(status);
      }
    }
  };

  bool found_full = false;
  for (int string_size = 0; string_size < 5; ++string_size) {
    EepromTlv::ClearAndInitializeEeprom(eeprom_);
    std::string std_string(string_size, 's');
    StringView s(std_string.data(), std_string.size());
    fill_function(s);
    if (absl::StrContains(PrintValueToStdString(eeprom_tlv_), "(Full)")) {
      found_full = true;
    }
  }
  EXPECT_TRUE(found_full);

  // Remove the entry with the tag, at which point the EEPROM should be empty.
  EXPECT_STATUS_OK(RemoveEntry(MCU_DOMAIN(1), 1));
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));
}

TEST_F(EepromTlvTest, ValidateWithCorruptPrefix) {
  eeprom_.write(0, 0);
  EXPECT_THAT(eeprom_tlv_.Validate(), StatusIs(StatusCode::kDataLoss))
      << PrintValueToStdString(eeprom_tlv_);
}

TEST_F(EepromTlvTest, ValidateWithWrongBeyondAddr) {
  EepromAddrT beyond_addr;
  eeprom_.get(kAddressOfFirstEntryAddress, beyond_addr);
  ++beyond_addr;
  eeprom_.put(kAddressOfFirstEntryAddress, beyond_addr);
  EXPECT_THAT(eeprom_tlv_.Validate(), StatusIs(StatusCode::kDataLoss))
      << PrintValueToStdString(eeprom_tlv_);
}

TEST_F(EepromTlvTest, ValidateWithWrongEntryLength) {
  constexpr EepromAddrT kAddressOfFirstEntry = 10;
  constexpr EepromAddrT kOffsetOfEntryDataLength = 2;

  // Write a first entry.
  EXPECT_STATUS_OK(WriteAndReadDouble(MCU_DOMAIN(TestDomain4), 2, 1.0));

  // Add one to its data length.
  EepromAddrT entry_data_length_addr =
      kAddressOfFirstEntry + kOffsetOfEntryDataLength;
  uint8_t entry_data_length;
  eeprom_.get(entry_data_length_addr, entry_data_length);
  EXPECT_EQ(entry_data_length, sizeof(double));
  ++entry_data_length;
  eeprom_.put(entry_data_length_addr, entry_data_length);

  // Should be invalid now.
  EXPECT_THAT(eeprom_tlv_.Validate(), StatusIs(StatusCode::kDataLoss));

  // And shouldn't be able to find the entry.
  EXPECT_THAT(FindEntry(MCU_DOMAIN(TestDomain4), 2),
              StatusIs(StatusCode::kDataLoss));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
