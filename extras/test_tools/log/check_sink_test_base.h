#ifndef MCUCORE_EXTRAS_TEST_TOOLS_LOG_CHECK_SINK_TEST_BASE_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_LOG_CHECK_SINK_TEST_BASE_H_

// Base class for tests that might trigger a failure in MCU_CHECK or MCU_DCHECK.

#include <functional>
#include <string>
#include <string_view>

#include "absl/strings/str_cat.h"
#include "extras/test_tools/print_to_std_string.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log_sink.h"

namespace mcucore {
namespace test {

class CheckSinkTestBase : public testing::Test {
 protected:
  void SetUp() override {
    LOG(INFO) << "CheckSinkTestBase::SetUp";
    SetCheckSinkExitFn(mock_exit_fn_.AsStdFunction());
    SetPrintForCheckSink(&out_);
  }

  void TearDown() override {
    SetCheckSinkExitFn(nullptr);
    SetPrintForCheckSink(nullptr);
  }

  // failing_func returns the line number from which the failure is expected to
  // be reported.
  void VerifyFailure(const std::function<int()>& failing_func,
                     std::string_view failure_file_name,
                     std::string_view expression_text,
                     std::string_view message_text,
                     bool verify_line_number = true) {
    out_.reset();
    EXPECT_CALL(mock_exit_fn_, Call("MCU_CHECK FAILED"));
    const int expected_line_number = failing_func();
    testing::Mock::VerifyAndClearExpectations(&mock_exit_fn_);
    std::string msg = out_.str();

    std::string expected_msg_prefix =
        absl::StrCat("MCU_CHECK FAILED: ", failure_file_name);
    ASSERT_THAT(msg, testing::StartsWith(expected_msg_prefix));
    if (verify_line_number) {
      absl::StrAppend(&expected_msg_prefix, ":", expected_line_number, "] ",
                      expression_text, " ", message_text, "\n");
      ASSERT_EQ(msg, expected_msg_prefix);
    } else {
      msg = msg.substr(expected_msg_prefix.size());
      // We don't know whether a line number appears, or if it does, what its
      // value is, so we just confirm that msg has the expected pattern after
      // the file name.
      ASSERT_THAT(msg, testing::ContainsRegex(R"re(^(:\d+)?\] )re"));
      // Remove text up to the "] ", then verify the remaining text.
      auto pos = msg.find("] ");
      ASSERT_NE(pos, std::string::npos);
      msg = msg.substr(pos);
      ASSERT_EQ(msg,
                absl::StrCat("] ", expression_text, " ", message_text, "\n"));
    }
  }

  PrintToStdString out_;
  testing::StrictMock<testing::MockFunction<void(std::string_view)>>
      mock_exit_fn_;
};

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_LOG_CHECK_SINK_TEST_BASE_H_
