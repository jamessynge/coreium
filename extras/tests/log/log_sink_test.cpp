#include "log/log_sink.h"

#include <functional>
#include <string_view>

#include "absl/strings/str_cat.h"
#include "extras/test_tools/print_to_std_string.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"
#include "print/o_print_stream.h"
#include "strings/progmem_string_data.h"

namespace mcucore {
namespace test {
namespace {

class LogSinkTest : public testing::Test {
 protected:
  void SetUp() override { SetPrintForLogSink(&out_); }
  void TearDown() override { SetPrintForLogSink(nullptr); }

  PrintToStdString out_;
};

TEST_F(LogSinkTest, LocationAndLine) {
  { LogSink sink(FLASHSTR("here"), 123); }
  EXPECT_EQ(out_.str(), "here:123] \n");
}

TEST_F(LogSinkTest, LocationAndZeroLine) {
  { LogSink sink(FLASHSTR("there.cc"), 0); }
  EXPECT_EQ(out_.str(), "there.cc] \n");
}

TEST_F(LogSinkTest, NullLocation) {
  // A null file means skip printing the location, so prints only a newline to
  // end the message.
  { LogSink sink(nullptr, 123); }
  EXPECT_EQ(out_.str(), "\n");
}

TEST_F(LogSinkTest, EmptyLocation) {
  // An empty file means skip printing the location, so prints only a newline to
  // end the message.
  { LogSink sink(FLASHSTR(""), 123); }
  EXPECT_EQ(out_.str(), "\n");
}

TEST_F(LogSinkTest, ZeroArgCtor) {
  { LogSink sink; }
  EXPECT_EQ(out_.str(), "\n");
}

TEST_F(LogSinkTest, InsertIntoNonTemporary) {
  {
    LogSink sink;
    sink << "abc";
  }
  EXPECT_EQ(out_.str(), "abc\n");
}

TEST_F(LogSinkTest, InsertIntoTemporary) {
  LogSink(FLASHSTR("whereelse"), 0) << "abc";
  EXPECT_EQ(out_.str(), "whereelse] abc\n");
}

class CheckSinkTest : public testing::Test {
 protected:
  void SetUp() override {
    SetCheckSinkExitFn(mock_exit_fn_.AsStdFunction());
    SetPrintForCheckSink(&out_);
    EXPECT_CALL(mock_exit_fn_, Call("MCU_CHECK FAILED")).Times(1);
  }
  void TearDown() override {
    SetCheckSinkExitFn(nullptr);
    SetPrintForCheckSink(nullptr);
  }

  PrintToStdString out_;
  testing::MockFunction<void(std::string_view)> mock_exit_fn_;
};

TEST_F(CheckSinkTest, LocationAndLineAndPrefix) {
  { CheckSink sink(FLASHSTR("foo.cc"), 123, FLASHSTR("prefix1")); }
  EXPECT_EQ(out_.str(), "MCU_CHECK FAILED: foo.cc:123] prefix1 \n");
}

TEST_F(CheckSinkTest, LocationAndLine) {
  { CheckSink sink(FLASHSTR("foo.cc"), 123, nullptr); }
  EXPECT_EQ(out_.str(), "MCU_CHECK FAILED: foo.cc:123] \n");
}

TEST_F(CheckSinkTest, LineAndPrefix) {
  { CheckSink sink(FLASHSTR(""), 123, FLASHSTR("Foo!=Bar")); }
  EXPECT_EQ(out_.str(), "MCU_CHECK FAILED: Foo!=Bar \n");
}

TEST_F(CheckSinkTest, LocationAndPrefix) {
  { CheckSink sink(FLASHSTR("bar.h"), 0, FLASHSTR("Bar!=Baz")); }
  EXPECT_EQ(out_.str(), "MCU_CHECK FAILED: bar.h] Bar!=Baz \n");
}

TEST_F(CheckSinkTest, InsertIntoNonTemporarySink) {
  {
    CheckSink sink(nullptr, 0, FLASHSTR("SomeExpr"));
    sink << "abc def";
  }
  EXPECT_EQ(out_.str(), "MCU_CHECK FAILED: SomeExpr abc def\n");
}

TEST_F(CheckSinkTest, InsertIntoTemporarySink) {
  { CheckSink(nullptr, 0, FLASHSTR("SomeExpr")) << "abc def"; }
  EXPECT_EQ(out_.str(), "MCU_CHECK FAILED: SomeExpr abc def\n");
}

// Since this is slow (needs to fork the process), I don't want to run it all
// the time. Given that I usually run tests with debugging enabled, only run it
// when debugging is not enabled or it is requested via a macro.
#if defined(NDEBUG) || defined(MCUCORE_EXECUTE_DEATH_TESTS)
TEST(CheckSinkDeathTest, CreateAndDelete) {
  EXPECT_DEATH(
      { CheckSink sink(FLASHSTR("my-file.cc"), 213, FLASHSTR("1!=2")); },
      "MCU_CHECK FAILED: my-file.cc:213] 1!=2");
}
#endif  // NDEBUG || MCUCORE_EXECUTE_DEATH_TESTS

}  // namespace
}  // namespace test
}  // namespace mcucore
