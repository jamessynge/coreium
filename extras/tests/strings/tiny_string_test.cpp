// Enable all of the logging features before including *any* files.
#include <string_view>
#undef MCU_DISABLE_CHECK
#undef MCU_DISABLE_CHECK_LOCATION
#undef MCU_DISABLE_DCHECK
#undef MCU_DISABLE_DCHECK_LOCATION
#undef MCU_DISABLE_VLOG
#undef MCU_DISABLE_VLOG_LOCATION

#define MCU_ENABLED_VLOG_LEVEL 5
#define MCU_ENABLE_CHECK
#define MCU_ENABLE_CHECK_LOCATION
#define MCU_ENABLE_DCHECK
#define MCU_ENABLE_DCHECK_LOCATION
#define MCU_ENABLE_VLOG
#define MCU_ENABLE_VLOG_LOCATION

#include <functional>
#include <string>

#include "extras/test_tools/log/check_sink_test_base.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/test_strings.h"
#include "gtest/gtest.h"
#include "strings/tiny_string.h"

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

class TinyStringCheckTest : public CheckSinkTestBase {
 protected:
  void VerifyDcheckFailure(const std::function<void()>& failing_func,
                           std::string_view expression_text,
                           std::string_view message_text) {
    VerifyFailure(
        [&]() {
          failing_func();
          return 0;
        },
        "tiny_string.h", expression_text, message_text,
        /*verify_line_number=*/false);
  }

 private:
  using CheckSinkTestBase::VerifyFailure;
};

TEST_F(TinyStringCheckTest, SetTooBig) {
  VerifyDcheckFailure(
      []() {
        TinyString<1> str;
        EXPECT_TRUE(str.Set("a", 1));
        EXPECT_EQ(PrintValueToStdString(str), "a");
        EXPECT_FALSE(str.Set("ab", 2));  // MCU_DCHECK_LE will fail.
        EXPECT_EQ(PrintValueToStdString(str), "");
      },
      "size <= N", "Too big");
}

TEST_F(TinyStringCheckTest, SetSizeTooBig) {
  VerifyDcheckFailure(
      []() {
        TinyString<1> str;
        EXPECT_TRUE(str.Set("a", 1));
        EXPECT_TRUE(str.set_size(0));
        EXPECT_TRUE(str.set_size(1));
        EXPECT_FALSE(str.set_size(2));  // MCU_DCHECK_LE will fail.
      },
      "size <= N", "Too big");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
