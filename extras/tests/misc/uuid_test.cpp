#include "misc/uuid.h"

#include <string>
#include <vector>

#include "eeprom/eeprom_tag.h"
#include "eeprom/eeprom_tlv.h"
#include "extras/host/eeprom/eeprom.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "status/status_code.h"

MCU_DEFINE_CONSTEXPR_NAMED_DOMAIN(TestDomain, 123);

namespace mcucore {
namespace test {
namespace {

using ::mcucore::PrintValueToStdString;

Uuid ZeroUuid() {
  Uuid uuid;
  uuid.Zero();
  return uuid;
}

Uuid RandomUuid() {
  Uuid uuid;
  uuid.Generate();
  return uuid;
}

bool IsZeroUuid(const Uuid& uuid) {
  if (uuid == ZeroUuid()) {
    CHECK_EQ(PrintValueToStdString(uuid),
             "00000000-0000-0000-0000-000000000000");
    return true;
  } else {
    CHECK_NE(PrintValueToStdString(uuid),
             "00000000-0000-0000-0000-000000000000");
    return false;
  }
}

TEST(UuidTest, Zero) {
  Uuid uuid;
  uuid.Zero();
  EXPECT_EQ(uuid, uuid);
  EXPECT_TRUE(IsZeroUuid(uuid));
}

TEST(UuidTest, GeneratedUuidsAreUnique) {
  const Uuid zero = ZeroUuid();

  const size_t kNumUuids = 100;
  std::vector<Uuid> uuids;
  uuids.reserve(kNumUuids);
  std::vector<std::string> uuid_strs;
  uuid_strs.reserve(kNumUuids);

  for (size_t i = 0; i < kNumUuids; ++i) {
    Uuid uuid;
    uuid.Generate();
    uuid_strs.push_back(PrintValueToStdString(uuid));
    EXPECT_EQ(PrintValueToStdString(uuid), uuid_strs[i]);

    EXPECT_EQ(uuid, uuid);
    EXPECT_NE(uuid, zero);

    // Make sure it is different from all the others.
    for (size_t j = 0; j < i; ++j) {
      EXPECT_NE(uuid, uuids[j]);
    }
    uuids.push_back(uuid);
    EXPECT_EQ(PrintValueToStdString(uuids[i]), uuid_strs[i]);
  }

  // The Uuids should have the same value, hence the same printed form, as
  // before.
  for (size_t i = 0; i < kNumUuids; ++i) {
    EXPECT_EQ(PrintValueToStdString(uuids[i]), uuid_strs[i]);
  }
}

TEST(UuidTest, WriteToReadFromEepromTlv) {
  EEPROMClass eeprom;
  ASSERT_STATUS_OK_AND_ASSIGN(EepromTlv tlv,
                              EepromTlv::ClearAndInitializeEeprom(eeprom));

  using UuidAndId = std::pair<Uuid, uint8_t>;
  std::vector<UuidAndId> uuid_and_ids = {
      {ZeroUuid(), 0},     {RandomUuid(), 1},   {RandomUuid(), 2},
      {RandomUuid(), 253}, {RandomUuid(), 254}, {ZeroUuid(), 255},
  };

  // Store them all.

  for (const auto& [uuid, id] : uuid_and_ids) {
    const auto tag = EepromTag{MCU_DOMAIN(TestDomain), id};

    Uuid missing;
    EXPECT_THAT(missing.ReadFromEeprom(tlv, tag),
                StatusIs(StatusCode::kNotFound));

    EXPECT_STATUS_OK(uuid.WriteToEeprom(tlv, tag));
  }

  // Read and verify them all.

  for (const auto& [uuid, id] : uuid_and_ids) {
    const auto tag = EepromTag{MCU_DOMAIN(TestDomain), id};
    Uuid stored;
    EXPECT_STATUS_OK(stored.ReadFromEeprom(tlv, tag));
    EXPECT_EQ(stored, uuid);
  }
}

TEST(UuidTest, ReadOrStoreEntry) {
  EEPROMClass eeprom;
  ASSERT_STATUS_OK_AND_ASSIGN(EepromTlv tlv,
                              EepromTlv::ClearAndInitializeEeprom(eeprom));

  using UuidAndTag = std::pair<Uuid, EepromTag>;
  std::vector<UuidAndTag> uuid_and_tags;

  // Store on first call.
  for (uint8_t id = 0; id < 10; ++id) {
    const auto tag = EepromTag{MCU_DOMAIN(TestDomain), id};
    {
      Uuid missing;
      EXPECT_THAT(missing.ReadFromEeprom(tlv, tag),
                  StatusIs(StatusCode::kNotFound));
    }
    Uuid uuid = ZeroUuid();
    EXPECT_STATUS_OK(uuid.ReadOrStoreEntry(tlv, tag));
    EXPECT_NE(uuid, ZeroUuid());
    uuid_and_tags.push_back({uuid, tag});

    // Try again, which should read the same value.
    {
      Uuid stored;
      EXPECT_STATUS_OK(stored.ReadFromEeprom(tlv, tag));
      EXPECT_EQ(stored, uuid);
    }
    {
      Uuid stored;
      EXPECT_STATUS_OK(stored.ReadOrStoreEntry(tlv, tag));
      EXPECT_EQ(stored, uuid);
    }
  }

  // Another loop, which should find the values stored already.
  for (const auto& [uuid, tag] : uuid_and_tags) {
    {
      Uuid stored;
      EXPECT_STATUS_OK(stored.ReadOrStoreEntry(tlv, tag));
      EXPECT_EQ(stored, uuid);
    }
    {
      Uuid stored;
      EXPECT_STATUS_OK(stored.ReadFromEeprom(tlv, tag));
      EXPECT_EQ(stored, uuid);
    }
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
