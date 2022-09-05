#include "container/serial_map.h"

#include <string>
#include <string_view>
#include <unordered_map>

#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "extras/test_tools/string_view_utils.h"
#include "extras/test_tools/test_has_failed.h"
#include "gtest/gtest.h"
#include "strings/progmem_string.h"
#include "strings/progmem_string_data.h"

namespace mcucore {
namespace test {
namespace {

template <typename SERIAL_MAP>
Status InsertStdStringView(SERIAL_MAP& serial_map, ProgmemString key,
                           std::string_view sv) {
  std::string str(sv);
  auto result = serial_map.Insert(key, str.length(),
                                  reinterpret_cast<const uint8_t*>(str.data()));
  // Overwrite str so we can be certain the serial_map doesn't somehow contain a
  // pointer to str's data.
  for (size_t pos = 0; pos < str.length(); ++pos) {
    str[pos] += 1;
  }
  return result;
}

template <typename SERIAL_MAP>
Status InsertOrAssignStdStringView(SERIAL_MAP& serial_map, ProgmemString key,
                                   std::string_view std_sv) {
  std::string str(std_sv);
  StringView sv = MakeStringView(str);
  auto result = serial_map.InsertOrAssign(key, sv);
  // Overwrite str so we can be certain the serial_map doesn't somehow contain a
  // pointer to str's data.
  for (size_t pos = 0; pos < str.length(); ++pos) {
    // Change some of the bits of the char.
    str[pos] += static_cast<char>(1 + (pos & 0x3f));
  }
  return result;
}

TEST(SerialMapTest, EmptyAtStart) {
  SerialMap<ProgmemString, 128> serial_map;
  EXPECT_EQ(serial_map.First(), nullptr);
  EXPECT_EQ(serial_map.Find(MCU_PSD("my key")), nullptr);
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("my key")));
  EXPECT_THAT(serial_map.GetValue<int>(MCU_PSD("my key")),
              StatusIs(StatusCode::kNotFound));
}

TEST(SerialMapTest, InsertAndRemoveFloat) {
  // An entry whose values are all of the same size.
  SerialMap<ProgmemString, 128> serial_map;
  EXPECT_STATUS_OK(serial_map.Insert(MCU_PSD("f"), 1.2f));

  EXPECT_THAT(serial_map.GetValue<float>(MCU_PSD("f")), IsOkAndHolds(1.2f));
  // Get as a StringView, which works because it can be of any length.
  EXPECT_STATUS_OK(serial_map.GetValue<StringView>(MCU_PSD("f")));
  // Get as a bool, which fails because it isn't the same size as a float.
  EXPECT_THAT(serial_map.GetValue<bool>(MCU_PSD("f")),
              StatusIs(StatusCode::kDataLoss));
  // No other keys are present.
  EXPECT_THAT(serial_map.GetValue<float>(MCU_PSD("")),
              StatusIs(StatusCode::kNotFound));
  EXPECT_THAT(serial_map.GetValue<int>(MCU_PSD("f2")),
              StatusIs(StatusCode::kNotFound));

  // If we attempt to remove another (non-existent) key, the inserted key
  // will still be present.
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("F")));
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("f2")));
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("")));
  EXPECT_THAT(serial_map.GetValue<float>(MCU_PSD("f")), IsOkAndHolds(1.2f));

  // Remove the float.
  EXPECT_TRUE(serial_map.Remove(MCU_PSD("f")));

  EXPECT_THAT(serial_map.GetValue<float>(MCU_PSD("f")),
              StatusIs(StatusCode::kNotFound));
  EXPECT_EQ(serial_map.First(), nullptr);
}

TEST(SerialMapTest, InsertAndRemoveStringView) {
  SerialMap<ProgmemString, 128> serial_map;
  // Insert the value, and confirm we can retrieve it.
  EXPECT_STATUS_OK(serial_map.Insert(MCU_PSD("sv"), StringView("some text")));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("sv")),
              IsOkAndHolds("some text"));

  // It's the first and only entry.
  {
    const auto* first = serial_map.First();
    ASSERT_NE(first, nullptr);
    EXPECT_THAT(first->GetKey(), MCU_PSD("sv"));
    EXPECT_THAT(first->GetValue<StringView>(), IsOkAndHolds("some text"));
    EXPECT_EQ(serial_map.Next(*first), nullptr);
  }

  // No other keys are present, regardless of type.
  EXPECT_THAT(serial_map.GetValue<int>(MCU_PSD("")),
              StatusIs(StatusCode::kNotFound));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("s")),
              StatusIs(StatusCode::kNotFound));
  EXPECT_THAT(serial_map.GetValue<int>(MCU_PSD("sV")),
              StatusIs(StatusCode::kNotFound));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("sv2")),
              StatusIs(StatusCode::kNotFound));

  // Can't request the entry as a type of the wrong size.
  EXPECT_THAT(serial_map.GetValue<bool>(MCU_PSD("sv")),
              StatusIs(StatusCode::kDataLoss));

  // Attempt to remove entries with similar keys, which will not touch the entry
  // that is present.
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("")));
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("s")));
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("sV")));
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("Sv")));
  EXPECT_FALSE(serial_map.Remove(MCU_PSD("sv2")));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("sv")),
              IsOkAndHolds("some text"));

  // Remove the entry, at which point the map will be empty.
  EXPECT_TRUE(serial_map.Remove(MCU_PSD("sv")));
  EXPECT_EQ(serial_map.First(), nullptr);
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("sv")),
              StatusIs(StatusCode::kNotFound));
  EXPECT_THAT(serial_map.GetValue<bool>(MCU_PSD("sv")),
              StatusIs(StatusCode::kNotFound));
}

TEST(SerialMapTest, InsertOnceOnly) {
  // Insert doesn't permit overwriting an existing entry.
  SerialMap<ProgmemString, 128> serial_map;
  EXPECT_STATUS_OK(InsertStdStringView(serial_map, MCU_PSD("key"), "abcdef"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key")),
              IsOkAndHolds("abcdef"));
  EXPECT_THAT(InsertStdStringView(serial_map, MCU_PSD("key"), "123"),
              StatusIs(StatusCode::kAlreadyExists, "Key in map"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key")),
              IsOkAndHolds("abcdef"));
}

TEST(SerialMapTest, InsertOrAssignSameSize) {
  SerialMap<ProgmemString, 128> serial_map;
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key0"), ""));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key1"), "A"));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key6"), "abcdef"));

  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key0")),
              IsOkAndHolds(""));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key1")),
              IsOkAndHolds("A"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key6")),
              IsOkAndHolds("abcdef"));

  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key0"), ""));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key1"), "B"));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key6"), "ABCDEF"));

  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key0")),
              IsOkAndHolds(""));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key1")),
              IsOkAndHolds("B"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key6")),
              IsOkAndHolds("ABCDEF"));
}

TEST(SerialMapTest, InsertOrAssignSmaller) {
  SerialMap<ProgmemString, 128> serial_map;
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key0"), "A"));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key1"), "AB"));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key6"), "abcdefg"));

  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key0")),
              IsOkAndHolds("A"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key1")),
              IsOkAndHolds("AB"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key6")),
              IsOkAndHolds("abcdefg"));

  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key0"), ""));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key1"), "b"));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key6"), "LMNOPQ"));

  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key0")),
              IsOkAndHolds(""));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key1")),
              IsOkAndHolds("b"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key6")),
              IsOkAndHolds("LMNOPQ"));
}

TEST(SerialMapTest, InsertOrAssignLarger) {
  SerialMap<ProgmemString, 128> serial_map;
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key0"), ""));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key1"), "A"));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key6"), "abcdef"));

  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key0")),
              IsOkAndHolds(""));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key1")),
              IsOkAndHolds("A"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key6")),
              IsOkAndHolds("abcdef"));

  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key0"), "X"));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key1"), "?!"));
  EXPECT_STATUS_OK(
      InsertOrAssignStdStringView(serial_map, MCU_PSD("key6"), "0123456"));

  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key0")),
              IsOkAndHolds("X"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key1")),
              IsOkAndHolds("?!"));
  EXPECT_THAT(serial_map.GetValue<StringView>(MCU_PSD("key6")),
              IsOkAndHolds("0123456"));
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

TEST(SerialMapTest, InsertAndFindSeveral) {
  // Multiple entries, of multiple types.
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

}  // namespace
}  // namespace test
}  // namespace mcucore
