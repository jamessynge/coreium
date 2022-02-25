// Enable all of the logging features before including *any* files.
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

#include "logging.h"

#include <functional>
#include <string>

#include "absl/strings/str_cat.h"
#include "extras/test_tools/print_to_std_string.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "progmem_string_view.h"

namespace mcucore {
namespace test {
namespace {

using ::mcucore::test::PrintToStdString;
using ::testing::_;
using ::testing::IsEmpty;

class McuVLogTest : public testing::Test {
 protected:
  void SetUp() override { SetPrintForLogSink(&out_); }
  void TearDown() override { SetPrintForLogSink(nullptr); }

  std::string TakeStr() {
    std::string str = out_.str();
    out_.reset();
    return str;
  }

  PrintToStdString out_;
};

TEST_F(McuVLogTest, VLogIsOn) {
  // Various McuVLogTest cases depend on this.
  ASSERT_EQ(MCU_ENABLED_VLOG_LEVEL, 5);

  EXPECT_TRUE(MCU_VLOG_IS_ON(1));
  EXPECT_TRUE(MCU_VLOG_IS_ON(2));
  EXPECT_TRUE(MCU_VLOG_IS_ON(3));
  EXPECT_TRUE(MCU_VLOG_IS_ON(4));
  EXPECT_TRUE(MCU_VLOG_IS_ON(5));

  EXPECT_FALSE(MCU_VLOG_IS_ON(6));
  EXPECT_FALSE(MCU_VLOG_IS_ON(7));
  EXPECT_FALSE(MCU_VLOG_IS_ON(8));
  EXPECT_FALSE(MCU_VLOG_IS_ON(9));
}

TEST_F(McuVLogTest, VLogLevelIsEnabled) {
  int line_before = __LINE__;
  MCU_VLOG(1) << "one";
  EXPECT_THAT(TakeStr(),
              absl::StrCat("logging_test.cc:", line_before + 1, "] one\n"));

  line_before = __LINE__;
  MCU_VLOG(2) << "two";
  EXPECT_THAT(TakeStr(),
              absl::StrCat("logging_test.cc:", line_before + 1, "] two\n"));

  line_before = __LINE__;
  MCU_VLOG(3) << "three";
  EXPECT_THAT(TakeStr(),
              absl::StrCat("logging_test.cc:", line_before + 1, "] three\n"));

  line_before = __LINE__;
  MCU_VLOG(4) << "four";
  EXPECT_THAT(TakeStr(),
              absl::StrCat("logging_test.cc:", line_before + 1, "] four\n"));

  line_before = __LINE__;
  MCU_VLOG(5) << "five";
  EXPECT_THAT(TakeStr(),
              absl::StrCat("logging_test.cc:", line_before + 1, "] five\n"));
}

TEST_F(McuVLogTest, VLogLevelIsDisabled) {
  MCU_VLOG(6) << 6;
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG(7) << 7.0;
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG(8) << 0x08;
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG(9) << 011;
  EXPECT_THAT(TakeStr(), IsEmpty());
}

TEST_F(McuVLogTest, NoMessage) {
  MCU_VLOG(9);
  EXPECT_THAT(TakeStr(), IsEmpty());

  int line_before = __LINE__;
  MCU_VLOG(1);
  EXPECT_THAT(TakeStr(),
              absl::StrCat("logging_test.cc:", line_before + 1, "] \n"));
}

TEST_F(McuVLogTest, MiscValues) {
  const int line_before = __LINE__;
  MCU_VLOG(1) << 'a' << "b" << MCU_PSV("c");
  EXPECT_THAT(TakeStr(),
              absl::StrCat("logging_test.cc:", line_before + 1, "] abc\n"));
}

TEST_F(McuVLogTest, VLogIfEnabledAndTrue) {
  auto true_function = [] { return true; };
  auto false_function = [] { return false; };

  int line_before = __LINE__;
  MCU_VLOG_IF(1, true) << "literal true";
  EXPECT_THAT(TakeStr(), absl::StrCat("logging_test.cc:", line_before + 1,
                                      "] literal true\n"));

  line_before = __LINE__;
  MCU_VLOG_IF(2, true != false) << "booleans expression";
  EXPECT_THAT(TakeStr(), absl::StrCat("logging_test.cc:", line_before + 1,
                                      "] booleans expression\n"));

  line_before = __LINE__;
  MCU_VLOG_IF(3, true_function()) << "function returning boolean";
  EXPECT_THAT(TakeStr(), absl::StrCat("logging_test.cc:", line_before + 1,
                                      "] function returning boolean\n"));

  line_before = __LINE__;
  MCU_VLOG_IF(4, &line_before) << "ptr is not null";
  EXPECT_THAT(TakeStr(), absl::StrCat("logging_test.cc:", line_before + 1,
                                      "] ptr is not null\n"));

  line_before = __LINE__;
  MCU_VLOG_IF(5, !false_function());
  EXPECT_THAT(TakeStr(),
              absl::StrCat("logging_test.cc:", line_before + 1, "] \n"));
}

TEST_F(McuVLogTest, VLogIfDisabledOrFalse) {
  auto true_function = [] { return true; };
  auto false_function = [] { return false; };

  // Through 5 the expression is false...
  MCU_VLOG_IF(1, false) << "literal false";
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG_IF(2, true != true) << "booleans expression";
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG_IF(3, false_function()) << "function returning boolean";
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG_IF(4, nullptr) << "ptr to boolean";
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG_IF(5, !true_function()) << "five";
  EXPECT_THAT(TakeStr(), IsEmpty());

  // From 6 the expression is true or false, but doesn't matter because disabled
  // at these levels.
  MCU_VLOG_IF(6, true) << 6;
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG_IF(7, true) << 7.0;
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG_IF(8, false) << 0x08;
  EXPECT_THAT(TakeStr(), IsEmpty());

  MCU_VLOG_IF(9, false) << 011;
  EXPECT_THAT(TakeStr(), IsEmpty());
}

class McuCheckTest : public testing::Test {
 protected:
  void SetUp() override {
    SetCheckSinkExitFn(mock_exit_fn_.AsStdFunction());
    SetPrintForCheckSink(&out_);
  }
  void TearDown() override {
    SetCheckSinkExitFn(nullptr);
    SetPrintForCheckSink(nullptr);
  }

  void VerifyFailure(std::function<int()> failing_func,
                     std::string_view expression_text,
                     std::string_view message_text) {
    out_.reset();
    std::string captured_str;
    EXPECT_CALL(mock_exit_fn_, Call(_))
        .WillOnce(testing::SaveArg<0>(&captured_str));
    const int line_number = failing_func();
    const auto common =
        absl::StrCat("MCU_CHECK FAILED: logging_test.cc:", line_number, "] ",
                     expression_text, " ");
    EXPECT_EQ(captured_str, common);
    EXPECT_EQ(out_.str(), absl::StrCat(common, message_text, "\n"));
  }

  PrintToStdString out_;
  testing::MockFunction<void(std::string_view)> mock_exit_fn_;
};

TEST_F(McuCheckTest, NonFailures) {
  auto fn = []() { return false; };
  EXPECT_CALL(mock_exit_fn_, Call(_)).Times(0);

  MCU_CHECK(fn() == false);
  MCU_CHECK_EQ(1 + 1, 2 + 0) << "Huh!";
  MCU_CHECK_NE(0.999, 1);
  MCU_CHECK_LT(0, 1);
  MCU_CHECK_LE(1, 1);
  MCU_CHECK_GE(1, 1);
  MCU_CHECK_GT(2, 1);

  MCU_DCHECK(fn() == false);
  MCU_DCHECK_EQ(1 + 1, 2 + 0) << "Huh!";
  MCU_DCHECK_NE(0.999, 1);
  MCU_DCHECK_LT(0, 1);
  MCU_DCHECK_LE(1, 1);
  MCU_DCHECK_GE(1, 1);
  MCU_DCHECK_GT(2, 1);

  EXPECT_EQ(out_.str(), "");
}

TEST_F(McuCheckTest, Failing_Check) {
  auto fn = []() { return false; };
  VerifyFailure(
      [&]() {
        MCU_CHECK(fn() == true) << "My Message";
        return __LINE__ - 1;
      },
      "fn() == true", "My Message");

  VerifyFailure(
      [&]() {
        MCU_DCHECK(fn() == true) << "MY MESSAGE";
        return __LINE__ - 1;
      },
      "fn() == true", "MY MESSAGE");
}

TEST_F(McuCheckTest, Failing_EQ) {
  VerifyFailure(
      [&]() {
        MCU_CHECK_EQ(false, true) << "SOME MESSAGE";
        return __LINE__ - 1;
      },
      "false == true", "SOME MESSAGE");

  VerifyFailure(
      [&]() {
        MCU_DCHECK_EQ(false, true) << "Some Message";
        return __LINE__ - 1;
      },
      "false == true", "Some Message");
}

TEST_F(McuCheckTest, Failing_NE) {
  VerifyFailure(
      [&]() {
        MCU_CHECK_NE(1, 1) << "A Message";
        return __LINE__ - 1;
      },
      "1 != 1", "A Message");

  VerifyFailure(
      [&]() {
        MCU_DCHECK_NE(1, 1) << "A Message";
        return __LINE__ - 1;
      },
      "1 != 1", "A Message");
}

TEST_F(McuCheckTest, Failing_LT) {
  VerifyFailure(
      [&]() {
        MCU_CHECK_LT(1, 0) << "my message";
        return __LINE__ - 1;
      },
      "1 < 0", "my message");

  VerifyFailure(
      [&]() {
        MCU_DCHECK_LT(1, 0) << "my message";
        return __LINE__ - 1;
      },
      "1 < 0", "my message");
}

TEST_F(McuCheckTest, Failing_LE) {
  VerifyFailure(
      [&]() {
        MCU_CHECK_LE(2, 1) << "another message";
        return __LINE__ - 1;
      },
      "2 <= 1", "another message");

  VerifyFailure(
      [&]() {
        MCU_DCHECK_LE(2, 1) << "another message";
        return __LINE__ - 1;
      },
      "2 <= 1", "another message");
}

TEST_F(McuCheckTest, Failing_GE) {
  VerifyFailure(
      [&]() {
        MCU_CHECK_GE(1, 2) << "some Message";
        return __LINE__ - 1;
      },
      "1 >= 2", "some Message");

  VerifyFailure(
      [&]() {
        MCU_DCHECK_GE(1, 2) << "some Message";
        return __LINE__ - 1;
      },
      "1 >= 2", "some Message");
}

TEST_F(McuCheckTest, Failing_GT) {
  VerifyFailure(
      [&]() {
        MCU_CHECK_GT(1, 1) << "Some MESSAGE";
        return __LINE__ - 1;
      },
      "1 > 1", "Some MESSAGE");

  VerifyFailure(
      [&]() {
        MCU_DCHECK_GT(1, 1) << "SOME message";
        return __LINE__ - 1;
      },
      "1 > 1", "SOME message");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
