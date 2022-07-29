#include "extras/host/arduino/wcharacter.h"

#include <limits>
#include <set>
#include <string_view>

// Tests of the functions like isAlphaNumeric. It might seem unnecessary for me
// to test these well documented functions, but the tests also help me confirm
// my understanding of them.

#include "extras/test_tools/print_value_to_std_string.h"
#include "gtest/gtest.h"

namespace {

TEST(RequestDecoderInternalsTest, isAlphaNumeric) {
  std::set<char> tested;
  for (char c = 'a'; c <= 'z'; ++c) {
    ASSERT_TRUE(isAlphaNumeric(c));
    ASSERT_TRUE(tested.insert(c).second);
  }
  for (char c = 'A'; c <= 'Z'; ++c) {
    ASSERT_TRUE(isAlphaNumeric(c));
    ASSERT_TRUE(tested.insert(c).second);
  }
  for (char c = '0'; c <= '9'; ++c) {
    ASSERT_TRUE(isAlphaNumeric(c));
    ASSERT_TRUE(tested.insert(c).second);
  }
  // Any char value not in tested should be non-alphanumeric.
  for (char c = std::numeric_limits<char>::min(); tested.size() < 256; ++c) {
    if (tested.count(c) == 0) {
      EXPECT_FALSE(isAlphaNumeric(c));
      ASSERT_TRUE(tested.insert(c).second);
    }
  }
}

TEST(RequestDecoderInternalsTest, isGraph) {
  std::set<char> tested;
  for (char c = 0x21; c < 127; ++c) {
    ASSERT_TRUE(isGraph(c));
    ASSERT_TRUE(tested.insert(c).second);
  }
  // Any char value not in tested should be non-graphic.
  for (char c = std::numeric_limits<char>::min(); tested.size() < 256; ++c) {
    if (tested.count(c) == 0) {
      EXPECT_FALSE(isGraph(c)) << mcucore::HexEscapedToStdString(c);
      ASSERT_TRUE(tested.insert(c).second);
    }
  }
}

TEST(RequestDecoderInternalsTest, isPrintable) {
  std::set<char> tested;
  for (char c = 0x20; c < 127; ++c) {
    ASSERT_TRUE(isPrintable(c));
    ASSERT_TRUE(tested.insert(c).second);
  }
  // Any char value not in tested should be non-printable.
  for (char c = std::numeric_limits<char>::min(); tested.size() < 256; ++c) {
    if (tested.count(c) == 0) {
      EXPECT_FALSE(isPrintable(c)) << mcucore::HexEscapedToStdString(c);
      ASSERT_TRUE(tested.insert(c).second);
    }
  }
}

TEST(RequestDecoderInternalsTest, isUpperCase) {
  std::set<char> tested;
  for (char c : std::string_view("ABCDEFGHIJKLMNOPQRSTUVWXYZ")) {
    ASSERT_TRUE(isUpperCase(c)) << mcucore::HexEscapedToStdString(c);
    ASSERT_TRUE(tested.insert(c).second);
  }
  // Any char value not in tested should not be uppercase.
  for (char c = std::numeric_limits<char>::min(); tested.size() < 256; ++c) {
    if (tested.count(c) == 0) {
      EXPECT_FALSE(isUpperCase(c)) << mcucore::HexEscapedToStdString(c);
      ASSERT_TRUE(tested.insert(c).second);
    }
  }
}

}  // namespace
