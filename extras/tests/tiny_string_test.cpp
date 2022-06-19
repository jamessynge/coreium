#include "tiny_string.h"

#include <string>

#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/test_strings.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

using ::mcucore::PrintValueToStdString;

TEST(TinyStringTest, MinimumTinyString) {
  TinyString<1> pico_str;
  const auto* data = pico_str.data();
  EXPECT_EQ(static_cast<const TinyString<1>&>(pico_str).data(), data);

  EXPECT_EQ(pico_str.maximum_size(), 1);
  EXPECT_EQ(pico_str.size(), 0);
  EXPECT_TRUE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), "");

  pico_str.Set("a", 1);

  EXPECT_EQ(pico_str.maximum_size(), 1);
  EXPECT_EQ(pico_str.size(), 1);
  EXPECT_FALSE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), "a");

  pico_str.set_size(0);

  EXPECT_EQ(pico_str.maximum_size(), 1);
  EXPECT_EQ(pico_str.size(), 0);
  EXPECT_TRUE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), "");

  pico_str.set_size(1);

  EXPECT_EQ(pico_str.maximum_size(), 1);
  EXPECT_EQ(pico_str.size(), 1);
  EXPECT_FALSE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), "a");

  EXPECT_EQ(pico_str.data(), data);
  EXPECT_EQ(static_cast<const TinyString<1>&>(pico_str).data(), data);
}

TEST(TinyStringTest, MaximumTinyString) {
  TinyString<255> pico_str;
  const auto* data = pico_str.data();
  EXPECT_EQ(static_cast<const TinyString<255>&>(pico_str).data(), data);

  EXPECT_EQ(pico_str.maximum_size(), 255);
  EXPECT_EQ(pico_str.size(), 0);
  EXPECT_TRUE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), "");

  pico_str.Set(TEST_STR_255, 255);
  const std::string kTestStr = TEST_STR_255;

  EXPECT_EQ(pico_str.maximum_size(), 255);
  EXPECT_EQ(pico_str.size(), 255);
  EXPECT_FALSE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), kTestStr);

  pico_str.set_size(0);

  EXPECT_EQ(pico_str.maximum_size(), 255);
  EXPECT_EQ(pico_str.size(), 0);
  EXPECT_TRUE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), "");

  pico_str.set_size(1);

  EXPECT_EQ(pico_str.maximum_size(), 255);
  EXPECT_EQ(pico_str.size(), 1);
  EXPECT_FALSE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), kTestStr.substr(0, 1));

  EXPECT_EQ(pico_str.data(), data);
  EXPECT_EQ(static_cast<const TinyString<255>&>(pico_str).data(), data);
}

#ifndef MCU_ENABLE_DCHECK

TEST(TinyStringDeathTest, SetTooBig) {
  TinyString<16> pico_str;

  EXPECT_FALSE(pico_str.Set(TEST_STR_17, 17));

  EXPECT_EQ(pico_str.maximum_size(), 16);
  EXPECT_EQ(pico_str.size(), 0);
  EXPECT_TRUE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), "");

  pico_str.Set(TEST_STR_16, 16);
  EXPECT_EQ(PrintValueToStdString(pico_str), TEST_STR_16);

  EXPECT_FALSE(pico_str.Set(TEST_STR_17, 17));

  EXPECT_EQ(pico_str.maximum_size(), 16);
  EXPECT_EQ(pico_str.size(), 0);
  EXPECT_TRUE(pico_str.empty());
  EXPECT_EQ(PrintValueToStdString(pico_str), "");
}

TEST(TinyStringDeathTest, SetSizeTooBig) {
  TinyString<16> pico_str;

  pico_str.Set(TEST_STR_14, 14);
  EXPECT_EQ(PrintValueToStdString(pico_str), TEST_STR_14);

  EXPECT_FALSE(pico_str.set_size(17));
  EXPECT_EQ(PrintValueToStdString(pico_str), TEST_STR_14);

  EXPECT_FALSE(pico_str.set_size(255));
  EXPECT_EQ(PrintValueToStdString(pico_str), TEST_STR_14);

  EXPECT_EQ(pico_str.maximum_size(), 16);
  EXPECT_EQ(pico_str.size(), 14);
  EXPECT_FALSE(pico_str.empty());
}

#endif

}  // namespace
}  // namespace test
}  // namespace mcucore
