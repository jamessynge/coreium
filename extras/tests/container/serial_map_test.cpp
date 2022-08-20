#include "container/serial_map.h"

#include <string>
#include <string_view>
#include <unordered_map>

#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "gtest/gtest.h"
#include "strings/progmem_string.h"
#include "strings/progmem_string_data.h"

namespace mcucore {
namespace test {
namespace {

template <typename SERIAL_MAP>
Status InsertStdStringView(SERIAL_MAP& serial_map, ProgmemString key,
                           std::string_view str) {
  return serial_map.Insert(key, str.length(),
                           reinterpret_cast<const uint8_t*>(str.data()));
}

TEST(SerialMapTest, EmptyAtStart) {
  SerialMap<ProgmemString, 128> serial_map;
  EXPECT_EQ(serial_map.First(), nullptr);
  EXPECT_EQ(serial_map.Find(MCU_PSD("my key")), nullptr);
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("my key")));
}

TEST(SerialMapTest, InsertOnceOnly) {
  SerialMap<ProgmemString, 128> serial_map;
  std::string some_data = "abcdef";
  EXPECT_STATUS_OK(InsertStdStringView(serial_map, MCU_PSD("key"), some_data));
  some_data =
      "ghijkl";  // Overwrite so it can't be the same string that is referenced.
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key")),
              IsOkAndHolds("abcdef"));

  EXPECT_THAT(InsertStdStringView(serial_map, MCU_PSD("key"), "123"),
              StatusIs(StatusCode::kAlreadyExists, "Key in map"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key")),
              IsOkAndHolds("abcdef"));
}

TEST(SerialMapTest, InsertAndFindSeveral) {
  SerialMap<ProgmemString, 128> serial_map;

  EXPECT_EQ(serial_map.Find(MCU_PSD("float")), nullptr);
  EXPECT_STATUS_OK(serial_map.Insert<float>(MCU_PSD("float"), 123.4f));
  EXPECT_THAT(serial_map.GetValue<float>(MCU_PSD("float")),
              IsOkAndHolds(123.4f));
  EXPECT_THAT(serial_map.Insert<float>(MCU_PSD("float"), 123.4f),
              StatusIs(StatusCode::kAlreadyExists));
  EXPECT_THAT(serial_map.GetValue<float>(MCU_PSD("float")),
              IsOkAndHolds(123.4f));

  EXPECT_EQ(serial_map.Find(MCU_PSD("bool")), nullptr);
  EXPECT_STATUS_OK(serial_map.Insert<bool>(MCU_PSD("bool"), true));
  EXPECT_THAT(serial_map.GetValue<bool>(MCU_PSD("bool")), IsOkAndHolds(true));
  EXPECT_THAT(serial_map.Insert<bool>(MCU_PSD("bool"), false),
              StatusIs(StatusCode::kAlreadyExists));
  EXPECT_THAT(serial_map.GetValue<bool>(MCU_PSD("bool")), IsOkAndHolds(true));

  std::string some_data = "AlphaBeta";
  EXPECT_STATUS_OK(InsertStdStringView(serial_map, MCU_PSD("str"), some_data));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("str")),
              IsOkAndHolds("AlphaBeta"));
  EXPECT_THAT(InsertStdStringView(serial_map, MCU_PSD("str"), "numbers"),
              StatusIs(StatusCode::kAlreadyExists, "Key in map"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("str")),
              IsOkAndHolds("AlphaBeta"));

  // Can't read as the wrong type if the sizes don't match.
  EXPECT_THAT(serial_map.GetValue<float>(MCU_PSD("bool")),
              StatusIs(StatusCode::kDataLoss));
  EXPECT_THAT(serial_map.GetValue<bool>(MCU_PSD("float")),
              StatusIs(StatusCode::kDataLoss));

  // Can read all the values previously inserted.

  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("str")),
              IsOkAndHolds("AlphaBeta"));
  EXPECT_THAT(serial_map.GetValue<float>(MCU_PSD("float")),
              IsOkAndHolds(123.4f));
  EXPECT_THAT(serial_map.GetValue<bool>(MCU_PSD("bool")), IsOkAndHolds(true));
}

TEST(SerialMapTest, Full) {
  SerialMap<ProgmemString, sizeof(ProgmemString) + 30> serial_map;
  std::string some_data = "abcdefghijklmnopqrstuvwxyz";
  EXPECT_STATUS_OK(InsertStdStringView(serial_map, MCU_PSD("key1"), some_data));
  EXPECT_THAT(InsertStdStringView(serial_map, MCU_PSD("key2"), some_data),
              StatusIs(StatusCode::kResourceExhausted, "Map too full"));
}

bool TestHasFailed() {
  auto test_info = testing::UnitTest::GetInstance()->current_test_info();
  return test_info->result()->Failed();
}

TEST(SerialMapTest, InsertOrAssignRepeatedly) {
  SerialMap<const void*, 254> serial_map;
  std::unordered_map<const void*, int> expected_contents;  // NOLINT

  auto verify_contents = [&]() {
    for (const auto& [key, value] : expected_contents) {
      ASSERT_THAT(serial_map.GetValue<int>(key), IsOkAndHolds(value)) << key;
    }
    for (auto* entry = serial_map.First(); entry != nullptr;
         entry = serial_map.Next(*entry)) {
      auto iter = expected_contents.find(entry->GetKey());
      ASSERT_NE(iter, expected_contents.end());
      ASSERT_THAT(entry->GetValue<int>(), IsOkAndHolds(iter->second));
    }
  };

  auto insert_or_assign = [&](const char* key, const int value) {
    serial_map.InsertOrAssign<int>(key, value);
    expected_contents[key] = value;
    verify_contents();
  };

  auto increment_key = [&](const char* key) {
    ASSERT_STATUS_OK_AND_ASSIGN(int value, serial_map.GetValue<int>(key));
    value += 3;
    serial_map.InsertOrAssign<int>(key, value);
    expected_contents[key] = value;
    verify_contents();
  };

  verify_contents();

  insert_or_assign("k1", 0);
  insert_or_assign("key2", 17);
  insert_or_assign("k1", 100);
  insert_or_assign("symbol3", 7);

  for (int i = 1; i <= 10 && !TestHasFailed(); ++i) {
    increment_key("key2");
    increment_key("k1");
    increment_key("symbol3");
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
