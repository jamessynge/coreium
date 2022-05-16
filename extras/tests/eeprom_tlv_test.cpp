#include "eeprom_tlv.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "eeprom_region.h"
#include "eeprom_tag.h"
#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "glog/logging.h"
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

inline std::ostream& operator<<(std::ostream& out, EepromTag tag) {
  return out << PrintValueToStdString(tag);
}

namespace test {
namespace {

using ::testing::AllOf;
using ::testing::AnyOf;
using ::testing::Gt;
using ::testing::HasSubstr;
using ::testing::Not;

constexpr EepromAddrT kAddressOfBeyondAddr = 4;
constexpr EepromAddrT kAddressOfCrc = 6;
constexpr EepromAddrT kAddressOfFirstEntry = 10;
constexpr EepromAddrT kOffsetOfEntryDataLength = 2;
constexpr EepromAddrT kSizeOfEntryHeader = 3;

EepromAddrT GetBeyondAddr(EEPROMClass& eeprom) {
  EepromAddrT beyond_addr;
  eeprom.get(kAddressOfBeyondAddr, beyond_addr);
  return beyond_addr;
}

EepromAddrT PutBeyondAddr(EEPROMClass& eeprom, const EepromAddrT beyond_addr) {
  return eeprom.put(kAddressOfBeyondAddr, beyond_addr);
}

EepromAddrT IncrementBeyondAddr(EEPROMClass& eeprom) {
  return PutBeyondAddr(eeprom, GetBeyondAddr(eeprom) + 1);
}

StringView ToStringView(std::string str) {
  return StringView(str.data(), str.length());
}

std::string GetStdString(EepromRegionReader& region) {
  std::string str(region.length(), 'X');
  EXPECT_TRUE(region.ReadString(str.data(), region.length()));
  return str;
}

std::string ToStdString(StringView view) {
  return std::string(view.data(), view.size());
}

TEST(EepromTlvGetIfValidTest, FailsWithNoPrefix) {
  EEPROMClass eeprom;
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kNotFound));

  // Get will call ClearAndInitializeEeprom not valid.
  ASSERT_STATUS_OK_AND_ASSIGN(auto eeprom_tlv, EepromTlv::Get(eeprom));
  EXPECT_STATUS_OK(eeprom_tlv.Validate());

  // Now there should be a prefix.
  ASSERT_STATUS_OK_AND_ASSIGN(auto eeprom_tlv2, EepromTlv::GetIfValid(eeprom));
  EXPECT_STATUS_OK(eeprom_tlv2.Validate());

  // Safe to call GetOrDie now.
  auto eeprom_tlv3 = EepromTlv::GetOrDie(eeprom);
  EXPECT_STATUS_OK(eeprom_tlv3.Validate());
}

TEST(EepromTlvGetIfValidTest, FailsWithZeroBeyondAddr) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  PutBeyondAddr(eeprom, EepromAddrT(0));
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kDataLoss));
}

TEST(EepromTlvGetIfValidTest, FailsWithTooLargeBeyondAddr) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  PutBeyondAddr(eeprom, EepromAddrT(65535));
  EXPECT_THAT(EepromTlv::GetIfValid(eeprom), StatusIs(StatusCode::kDataLoss));
}

TEST(EepromTlvGetIfValidTest, InitializeEepromFirst) {
  EEPROMClass eeprom;
  EepromTlv::ClearAndInitializeEeprom(eeprom);
  auto status_or_eeprom_tlv = EepromTlv::GetIfValid(eeprom);
  ASSERT_STATUS_OK(status_or_eeprom_tlv.status());
}

class UnreadableEepromClass : public EEPROMClass {
 public:
  uint8_t read(int idx) override { return 0; }
};

TEST(EepromTlvGetOrDieDeathTest, DiesWithNoPrefix) {
  UnreadableEepromClass eeprom;
  // The error depends on whether MCU_DCHECK is enabled or not. Cover both
  // cases.
  EXPECT_DEATH(
      EepromTlv::GetOrDie(eeprom),
      AnyOf(HasSubstr("IsPrefixPresent"), HasSubstr("TLV Prefix missing")));
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

// Captures the start address of the data region in start_address.
Status WriteStringFn(EepromRegion& region, const StringView value,
                     EepromAddrT& start_address) {
  start_address = region.start_address();
  if (region.WriteString(value)) {
    return OkStatus();
  } else {
    return ResourceExhaustedError();
  }
}

// Captures the start address of the data region in start_address.
Status AttemptNestedWriteStringTransactionFn(EepromRegion& region,
                                             EepromTlv& eeprom_tlv,
                                             const EepromTag nested_tag,
                                             const StringView outer_value,
                                             const StringView inner_value,
                                             EepromAddrT& start_address) {
  {
    const Status status = WriteStringFn(region, outer_value, start_address);
    EXPECT_STATUS_OK(status);
    MCU_RETURN_IF_ERROR(status);
  }
  LOG(INFO) << "AttemptNestedWriteStringTransactionFn with tag " << nested_tag;
  EepromAddrT nested_start_address = 12345;
  auto status = eeprom_tlv.WriteEntryToCursor(nested_tag, inner_value.size(),
                                              WriteStringFn, inner_value,
                                              nested_start_address);
  EXPECT_THAT(status, StatusIs(StatusCode::kFailedPrecondition,
                               HasSubstr("Write in progress")));
  // Shouldn't have recorded start address of nested transaction.
  EXPECT_EQ(nested_start_address, 12345);
  if (!IsFailedPrecondition(status)) {
    return UnknownError(MCU_PSV("Nested transaction unexpectedly succeeded"));
  }
  return OkStatus();
}

// Captures the start address of the data region in start_address.
Status WriteAlmostFullStringFn(EepromRegion& region,
                               const uint8_t room_to_leave,
                               EepromAddrT& start_address) {
  LOG(INFO) << "WriteAlmostFullStringFn: room_to_leave=" << (room_to_leave + 0)
            << ", start_address=" << region.start_address()
            << ", length=" << region.length();
  if (room_to_leave > region.length()) {
    return ResourceExhaustedError();
  }
  start_address = region.start_address();
  std::string s(region.length() - room_to_leave, 'V');
  LOG(INFO) << "WriteAlmostFullStringFn: s=" << s;
  StringView value(s.data(), s.length());
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

  auto FindEntry(EepromDomain domain, uint8_t id) {
    EepromTag tag{domain, id};
    return eeprom_tlv_.FindEntry(tag);
  }

  // Using strings here as a convenient way to express values that are of
  // specified lengths. Returns the start address of the data region of the
  // entry if successful.
  StatusOr<EepromAddrT> WriteStringEntryToCursor(EepromDomain domain,
                                                 uint8_t id,
                                                 const StringView value) {
    MCU_CHECK_OK(eeprom_tlv_.Validate());
    EepromTag tag{domain, id};
    EepromAddrT start_address;
    MCU_RETURN_IF_ERROR(eeprom_tlv_.WriteEntryToCursor(
        tag, value.size(), WriteStringFn, value, start_address));
    return start_address;
  }
  StatusOr<EepromAddrT> WriteStringEntryToCursor(EepromDomain domain,
                                                 uint8_t id,
                                                 const std::string& value) {
    return WriteStringEntryToCursor(domain, id, ToStringView(value));
  }

  StatusOr<std::pair<EepromAddrT, std::string>> ReadLocationAndStdString(
      const EepromDomain domain, const uint8_t id) {
    MCU_RETURN_IF_ERROR(eeprom_tlv_.Validate());
    EepromTag tag{domain, id};
    MCU_ASSIGN_OR_RETURN(auto entry_region, eeprom_tlv_.FindEntry(tag));
    return std::make_pair(entry_region.start_address(),
                          GetStdString(entry_region));
  }

  StatusOr<std::string> ReadStdString(const EepromDomain domain,
                                      const uint8_t id) {
    MCU_ASSIGN_OR_RETURN(const auto read_result,
                         ReadLocationAndStdString(domain, id));
    return read_result.second;
  }

  // Writes an entry with a string as the value, reads it back and verifies that
  // the expected entry and value are found. Returns the start address of the
  // data region of the entry if successful.
  StatusOr<EepromAddrT> WriteAndReadStdString(const EepromDomain domain,
                                              const uint8_t id,
                                              const std::string& value) {
    MCU_ASSIGN_OR_RETURN(const auto start_address_of_write,
                         WriteStringEntryToCursor(domain, id, value));
    MCU_ASSIGN_OR_RETURN(const auto read_result,
                         ReadLocationAndStdString(domain, id));
    EXPECT_EQ(start_address_of_write, read_result.first);
    EXPECT_EQ(read_result.second, value);
    return read_result.first;
  }

  auto DeleteEntry(EepromDomain domain, uint8_t id) {
    EepromTag tag{domain, id};
    return eeprom_tlv_.DeleteEntry(tag);
  }

  EEPROMClass eeprom_;
  EepromTlv eeprom_tlv_;
};

namespace {

TEST_F(EepromTlvTest, LotsAvailable) {
  EXPECT_EQ(eeprom_tlv_.Available(), eeprom_.length() -
                                         EepromTlv::kFixedHeaderSize -
                                         EepromTlv::kEntryHeaderSize);
  // There are no entries, so no space to reclaim.
  EXPECT_THAT(eeprom_tlv_.ReclaimUnusedSpace(), IsOkAndHolds(0));
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_),
              AllOf(HasSubstr("Prefix:OK"), HasSubstr("(Empty)"),
                    Not(HasSubstr("Entry@")), Not(HasSubstr("MISALIGNED"))));
}

TEST_F(EepromTlvTest, FindEntryNotFound) {
  for (const auto domain : {MCU_DOMAIN(1), MCU_DOMAIN(TestDomain2),
                            MCU_DOMAIN(3), MCU_DOMAIN(TestDomain4)}) {
    for (int id = 0; id <= 255; ++id) {
      EepromTag tag{domain, static_cast<uint8_t>(id)};
      EXPECT_THAT(eeprom_tlv_.FindEntry(tag), StatusIs(StatusCode::kNotFound));
    }
  }
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));
}

TEST_F(EepromTlvTest, WriteAndReadOneEmptyString) {
  const auto first_available = eeprom_tlv_.Available();
  EXPECT_STATUS_OK(WriteAndReadStdString(MCU_DOMAIN(1), 1, ""));
  EXPECT_EQ(first_available - kSizeOfEntryHeader, eeprom_tlv_.Available());
  // There's an entry, but no space to reclaim.
  EXPECT_THAT(eeprom_tlv_.ReclaimUnusedSpace(), IsOkAndHolds(0));
}

TEST_F(EepromTlvTest, VariousStringEntries) {
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));

  // Write some entries.
  std::vector<std::pair<std::string, std::string>> values = {
      {"", "Alpha"},       {"Bravo", "Charlie"}, {"Delta", "Echo"},
      {"Foxtrot", "Golf"}, {"Hotel", "India"},   {"Juliet", "Kilo"},
      {"Lima", "Mike"},    {"November", ""},
  };
  uint8_t id = 0;
  for (const auto& value : values) {
    ++id;
    EXPECT_THAT(FindEntry(MCU_DOMAIN(1), id), StatusIs(StatusCode::kNotFound));
    EXPECT_STATUS_OK(WriteAndReadStdString(MCU_DOMAIN(1), id, value.first));
    EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), Not(HasSubstr("(Empty)")));
  }

  // There are multiple entries, but no space to reclaim.
  ASSERT_THAT(eeprom_tlv_.ReclaimUnusedSpace(), IsOkAndHolds(0));
  MCU_VLOG(1) << "Reclaimed no space.";

  // Read each entry, then replace it.
  id = 0;
  for (const auto& value : values) {
    ++id;
    const auto expected_value = value.first;
    ASSERT_THAT(ReadStdString(MCU_DOMAIN(1), id), IsOkAndHolds(expected_value));
    EXPECT_STATUS_OK(WriteAndReadStdString(MCU_DOMAIN(1), id, value.second));
    PrintValueToStdString(eeprom_tlv_);
  }

  // There are multiple entries, and one run of unused entries.
  ASSERT_THAT(eeprom_tlv_.ReclaimUnusedSpace(), IsOkAndHolds(Gt(0)));
  // There should now be no unused space
  EXPECT_THAT(eeprom_tlv_.ReclaimUnusedSpace(), IsOkAndHolds(0));

  // Read each replaced entry, then remove it.
  id = 0;
  for (const auto& value : values) {
    ++id;
    const auto expected_value = value.second;
    ASSERT_THAT(ReadStdString(MCU_DOMAIN(1), id), IsOkAndHolds(expected_value));
    EepromTag tag{MCU_DOMAIN(1), id};
    ASSERT_STATUS_OK(eeprom_tlv_.DeleteEntry(tag));
    EXPECT_THAT(FindEntry(MCU_DOMAIN(1), id), StatusIs(StatusCode::kNotFound));
    PrintValueToStdString(eeprom_tlv_);
  }

  // The printed debug info should include the string "(Empty)", because
  // removing the last entry should cause a reset back to the "just initialized"
  // state.
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));
}

TEST_F(EepromTlvTest, FillEepromWithDistinctEntries) {
  // Fills the EEPROM with the same string entry again and again; once full,
  // shrink the string until we're able to write some more or the string is
  // empty.
  auto fill_function = [&](StringView s) {
    for (int i = 0; i < eeprom_.length(); ++i) {
      ASSERT_STATUS_OK(eeprom_tlv_.Validate());
      Status status = WriteStringEntryToCursor(MCU_DOMAIN(1), i, s);
      if (IsResourceExhausted(status)) {
        if (s.empty()) {
          EXPECT_EQ(eeprom_tlv_.Available(), 0);
          return;
        } else {
          s.remove_prefix(1);
          i -= 1;
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
}

TEST_F(EepromTlvTest, FillEepromWithReclaim) {
  // Fills the EEPROM with the same string entry again and again.
  // WriteEntryToCursor will automatically reclaim unused entries, i.e. the
  // prior entries with the same tag that have been marked as invalid.

  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));

  EepromAddrT last_address = 0;
  bool has_wrapped = false;
  for (int i = 0; i < eeprom_.length(); ++i) {
    auto data = absl::StrCat(">> ", i, " <<");
    ASSERT_STATUS_OK(eeprom_tlv_.Validate());
    ASSERT_STATUS_OK_AND_ASSIGN(auto this_address,
                                WriteAndReadStdString(MCU_DOMAIN(1), 1, data));
    EXPECT_NE(this_address, last_address);
    if (this_address < last_address) {
      has_wrapped = true;
    }
    last_address = this_address;

    EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), Not(HasSubstr("(Empty)")));
  }
  EXPECT_TRUE(has_wrapped);

  // Remove the entry with the tag, at which point the EEPROM should be empty.
  EXPECT_STATUS_OK(DeleteEntry(MCU_DOMAIN(1), 1));
  EXPECT_THAT(eeprom_tlv_.ReclaimUnusedSpace(), IsOkAndHolds(0));
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));

  // Can't delete the entry twice.
  EXPECT_THAT(DeleteEntry(MCU_DOMAIN(1), 1), StatusIs(StatusCode::kNotFound));
}

TEST_F(EepromTlvTest, ValidateWithCorruptPrefix) {
  eeprom_.write(0, 0);
  EXPECT_THAT(eeprom_tlv_.Validate(), StatusIs(StatusCode::kDataLoss));
  EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("Prefix:Missing"));
}

TEST_F(EepromTlvTest, ValidateWithWrongBeyondAddr) {
  PutBeyondAddr(eeprom_, GetBeyondAddr(eeprom_) - 1);
  EXPECT_THAT(eeprom_tlv_.Validate(), StatusIs(StatusCode::kDataLoss));
  EXPECT_THAT(
      PrintValueToStdString(eeprom_tlv_),
      HasSubstr(absl::StrCat("Beyond=", GetBeyondAddr(eeprom_), " (Invalid)")));
}

TEST_F(EepromTlvTest, CorruptBeyondAddr) {
  PutBeyondAddr(eeprom_, 1);
  EXPECT_THAT(eeprom_tlv_.Validate(), StatusIs(StatusCode::kDataLoss));
  EXPECT_EQ(eeprom_tlv_.Available(), 0);
}

TEST_F(EepromTlvTest, ValidateWithWrongEntryLength) {
  // Write a first entry.
  std::string value = "abcdefgh";
  EXPECT_STATUS_OK(WriteAndReadStdString(MCU_DOMAIN(TestDomain4), 2, value));

  // Add one to its data length.
  EepromAddrT entry_data_length_addr =
      kAddressOfFirstEntry + kOffsetOfEntryDataLength;
  uint8_t entry_data_length;
  eeprom_.get(entry_data_length_addr, entry_data_length);
  EXPECT_EQ(entry_data_length, value.size());
  ++entry_data_length;
  eeprom_.put(entry_data_length_addr, entry_data_length);

  // Should be invalid now.
  EXPECT_THAT(eeprom_tlv_.Validate(), StatusIs(StatusCode::kDataLoss));

  // And shouldn't be able to find the entry.
  EXPECT_THAT(FindEntry(MCU_DOMAIN(TestDomain4), 2),
              StatusIs(StatusCode::kDataLoss));
}

TEST_F(EepromTlvTest, ValidateWithCorruptEntryLength) {
  // Fill the EEPROM with entries that are as large as possible.
  uint8_t minimum_size = 255;
  uint8_t id = 0;
  const uint8_t kLeaveRoom = 1;
  EepromAddrT last_start_address = 0;
  uint8_t last_id = 0;
  bool did_reduce_size = false;
  while (true) {
    MCU_CHECK_OK(eeprom_tlv_.Validate());
    EepromTag tag{MCU_DOMAIN(1), id};
    EepromAddrT start_address = 0;
    const auto status = eeprom_tlv_.WriteEntryToCursor(
        tag, minimum_size, WriteAlmostFullStringFn, kLeaveRoom, start_address);
    if (status.ok()) {
      last_start_address = start_address;
      last_id = id;
      if (did_reduce_size) {
        // We've had to reduce the size to fit, so we should have filled it.
        break;
      }
      ++id;
      continue;
    } else if (IsResourceExhausted(status)) {
      ASSERT_GT(minimum_size, kLeaveRoom);
      --minimum_size;
      did_reduce_size = true;
      continue;
    } else {
      //
      ASSERT_STATUS_OK(status)
          << "Should have been OK or resource exhausted. Why wasn't it?";
    }
  }
  ASSERT_GT(last_start_address, 0);

  // Confirm we can find every successfully stored entry.
  for (id = 0; id <= last_id; ++id) {
    ASSERT_STATUS_OK(FindEntry(MCU_DOMAIN(1), id));
  }

  // Now corrupt the EEPROM by increasing the size of the last entry.
  uint8_t length;
  eeprom_.get(last_start_address - 1, length);
  EXPECT_EQ(length, minimum_size - kLeaveRoom);
  length = 255;
  eeprom_.put(last_start_address - 1, length);

  // We should no longer be able to read any entry.
  for (id = 0; id <= last_id; ++id) {
    EXPECT_THAT(FindEntry(MCU_DOMAIN(1), id), StatusIs(StatusCode::kDataLoss));
  }
}

TEST_F(EepromTlvTest, ReservedDomain) {
  // This needs to use internal::MakeEepromDomain because we can't define
  // reserved domains.
  EXPECT_THAT(
      WriteStringEntryToCursor(internal::MakeEepromDomain(0), 1, ""),
      StatusIs(StatusCode::kInvalidArgument, HasSubstr("Domain is reserved")));
  EXPECT_THAT(
      WriteStringEntryToCursor(internal::MakeEepromDomain(255), 1, "abc"),
      StatusIs(StatusCode::kInvalidArgument, HasSubstr("Domain is reserved")));
}

TEST_F(EepromTlvTest, NestedTransactionFails) {
  const EepromTag outer_tag{MCU_DOMAIN(1), 1};
  const EepromTag nested_tag{MCU_DOMAIN(1), 2};
  const StringView outer_value("abc");
  const StringView inner_value("DEF");
  EepromAddrT start_address;
  ASSERT_STATUS_OK(eeprom_tlv_.WriteEntryToCursor(
      outer_tag, sizeof outer_value, AttemptNestedWriteStringTransactionFn,
      eeprom_tlv_, nested_tag, outer_value, inner_value, start_address));
  EXPECT_EQ(start_address, kAddressOfFirstEntry + kSizeOfEntryHeader);
}

Status CorruptBeyondAddrFn(EepromRegion& region, EEPROMClass& eeprom) {
  PutBeyondAddr(eeprom, GetBeyondAddr(eeprom) + 1);
  return OkStatus();
}

TEST_F(EepromTlvTest, BeyondAddrCorruptedDuringTransaction) {
  const EepromTag tag{MCU_DOMAIN(1), 1};
  ASSERT_THAT(
      eeprom_tlv_.WriteEntryToCursor(tag, 1, CorruptBeyondAddrFn, eeprom_),
      StatusIs(StatusCode::kInternal, HasSubstr("Commit wrong data_addr")));
}

Status FailToWriteFn(EepromRegion& region) {
  return UnknownError(MCU_PSV("FailToWriteFn"));
}

TEST_F(EepromTlvTest, AbortTransaction) {
  const EepromTag tag{MCU_DOMAIN(1), 1};
  ASSERT_THAT(eeprom_tlv_.WriteEntryToCursor(tag, 1, FailToWriteFn),
              StatusIs(StatusCode::kUnknown, "FailToWriteFn"));
}

class DeleteSelectedEntriesTest : public EepromTlvTest {
 protected:
  static std::string MakeData(int index) {
    return absl::StrCat(">>>>>>>>>> ", index, " <<<<<<<<<<<<");
  }

  StatusOr<int> FillEeprom() {
    // Fill the EEPROM.
    for (int i = 0; i < 256; ++i) {
      MCU_RETURN_IF_ERROR(eeprom_tlv_.Validate());
      const auto data = MakeData(i);
      auto status = WriteAndReadStdString(MCU_DOMAIN(1), i, data);
      EXPECT_STATUS_OK(eeprom_tlv_.Validate());
      if (status.ok()) {
        continue;
      }
      EXPECT_THAT(status, StatusIs(StatusCode::kResourceExhausted));
      if (IsResourceExhausted(status)) {
        EXPECT_GT(i, 2);
        return i - 1;
      } else {
        return status.status();
      }
    }
    ADD_FAILURE() << "Should have ended before i == 256";
    return UnknownError();
  }

  void ConfirmExpectedContents(
      int maximum_index,
      const std::function<bool(int index, int maximum_index)>& delete_policy) {
    ASSERT_STATUS_OK(eeprom_tlv_.Validate());
    for (int i = 0; i <= maximum_index; ++i) {
      if (delete_policy(i, maximum_index)) {
        ASSERT_THAT(ReadStdString(MCU_DOMAIN(1), i),
                    StatusIs(StatusCode::kNotFound));
      } else {
        ASSERT_THAT(ReadStdString(MCU_DOMAIN(1), i), IsOkAndHolds(MakeData(i)));
      }
    }
    ASSERT_STATUS_OK(eeprom_tlv_.Validate());
  }

  void DeleteSelectedEntries(
      int maximum_index,
      const std::function<bool(int index, int maximum_index)>& delete_policy) {
    ASSERT_STATUS_OK(eeprom_tlv_.Validate());
    for (int i = 0; i <= maximum_index; ++i) {
      if (delete_policy(i, maximum_index)) {
        ASSERT_STATUS_OK(DeleteEntry(MCU_DOMAIN(1), i));
        ASSERT_STATUS_OK(eeprom_tlv_.Validate());
      }
    }
  }

  void FillEepromAndDeleteSomeEntriesAndReclaim(
      const std::function<bool(int index, int maximum)>& delete_policy) {
    // Fills the EEPROM with entries with different tags and values until full.

    EXPECT_THAT(PrintValueToStdString(eeprom_tlv_), HasSubstr("(Empty)"));
    ASSERT_STATUS_OK_AND_ASSIGN(const int maximum_index, FillEeprom());
    (void)maximum_index;
    ConfirmExpectedContents(maximum_index,
                            [](int index, int maximum_index) { return false; });
    DeleteSelectedEntries(maximum_index, delete_policy);
    ConfirmExpectedContents(maximum_index, delete_policy);

    // Reclaim the deleted entries.
    ASSERT_THAT(eeprom_tlv_.ReclaimUnusedSpace(), IsOkAndHolds(Gt(0)));
    ASSERT_STATUS_OK(eeprom_tlv_.Validate());

    // There should now be no unused space
    ASSERT_THAT(eeprom_tlv_.ReclaimUnusedSpace(), IsOkAndHolds(0));
    ASSERT_STATUS_OK(eeprom_tlv_.Validate());

    ConfirmExpectedContents(maximum_index, delete_policy);
  }
};

TEST_F(DeleteSelectedEntriesTest, DeleteOddEntries) {
  FillEepromAndDeleteSomeEntriesAndReclaim(
      [](int index, int maximum_index) -> bool {
        return index != 0 && index != maximum_index && (index % 2) == 1;
      });
}

TEST_F(DeleteSelectedEntriesTest, DeleteEvenEntries) {
  FillEepromAndDeleteSomeEntriesAndReclaim(
      [](int index, int maximum_index) -> bool {
        return index == 0 || index == maximum_index || (index % 2) == 0;
      });
}

TEST_F(DeleteSelectedEntriesTest, DeleteFirstAndLastEntries) {
  FillEepromAndDeleteSomeEntriesAndReclaim(
      [](int index, int maximum_index) -> bool {
        return index == 0 || index == maximum_index;
      });
}

TEST_F(DeleteSelectedEntriesTest, KeepFirstAndLastEntries) {
  FillEepromAndDeleteSomeEntriesAndReclaim(
      [](int index, int maximum_index) -> bool {
        return !(index == 0 || index == maximum_index);
      });
}

}  // namespace
}  // namespace test
}  // namespace mcucore
