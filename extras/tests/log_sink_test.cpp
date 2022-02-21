#include "log_sink.h"

#include <functional>
#include <string_view>

#include "absl/strings/str_cat.h"
#include "extras/test_tools/print_to_std_string.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"
#include "o_print_stream.h"
#include "progmem_string_data.h"

namespace mcucore {
namespace test {
namespace {

class LogSinkTest : public testing::Test {
 protected:
  void SetUp() override { SetPrintForLogSink(&out_); }
  void TearDown() override { SetPrintForLogSink(nullptr); }

  PrintToStdString out_;
};

TEST_F(LogSinkTest, ExplicitPrint) {
  {
    PrintToStdString other;
    { LogSink sink(other, FLASHSTR("here"), 123); }
    EXPECT_EQ(other.str(), "here:123] \n");
  }
  {
    // Omits the line number if it is zero.
    PrintToStdString other;
    { LogSink sink(other, FLASHSTR("there"), 0); }
    EXPECT_EQ(other.str(), "there] \n");
  }
  // An empty or null file means skip printing the location, so prints only a
  // newline to end the message.
  {
    PrintToStdString other;
    { LogSink sink(other, nullptr, 123); }
    EXPECT_EQ(other.str(), "\n");
  }
  {
    PrintToStdString other;
    { LogSink sink(other, FLASHSTR(""), 123); }
    EXPECT_EQ(other.str(), "\n");
  }
  {
    PrintToStdString other;
    // Omit the file ane line number ars entirely.
    { LogSink sink(other); }
    // Prints only a newline to end the message.
    EXPECT_EQ(other.str(), "\n");
  }
}

TEST_F(LogSinkTest, NoArgCtor) {
  { LogSink sink; }
  EXPECT_EQ(out_.str(), "\n");
}

TEST_F(LogSinkTest, ImplicitPrint) {
  { LogSink sink(FLASHSTR("somewhere"), 1); }
  EXPECT_EQ(out_.str(), "somewhere:1] \n");
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
  }
  void TearDown() override {
    SetCheckSinkExitFn(nullptr);
    SetPrintForCheckSink(nullptr);
  }

  PrintToStdString out_;
  testing::MockFunction<void(std::string_view)> mock_exit_fn_;
};

TEST_F(CheckSinkTest, CreateAndDelete) {
  const std::string_view common = "MCU_CHECK FAILED: foo.cc:123] prefix1 ";
  EXPECT_CALL(mock_exit_fn_, Call(common)).Times(1);
  { CheckSink sink(FLASHSTR("foo.cc"), 123, FLASHSTR("prefix1")); }
  EXPECT_EQ(out_.str(), absl::StrCat(common, "\n"));
}

TEST_F(CheckSinkTest, NoExpressionMessage) {
  const std::string_view common = "MCU_CHECK FAILED: foo.cc:123] ";
  EXPECT_CALL(mock_exit_fn_, Call(common)).Times(1);
  { CheckSink sink(FLASHSTR("foo.cc"), 123, nullptr); }
  EXPECT_EQ(out_.str(), absl::StrCat(common, "\n"));
}

TEST_F(CheckSinkTest, NoFileName) {
  const std::string_view common = "MCU_CHECK FAILED: Foo!=Bar ";
  EXPECT_CALL(mock_exit_fn_, Call(common)).Times(1);
  { CheckSink sink(FLASHSTR(""), 123, FLASHSTR("Foo!=Bar")); }
  EXPECT_EQ(out_.str(), absl::StrCat(common, "\n"));
}

TEST_F(CheckSinkTest, NoLineNumber) {
  const std::string_view common = "MCU_CHECK FAILED: bar.h] Bar!=Baz ";
  EXPECT_CALL(mock_exit_fn_, Call(common)).Times(1);
  { CheckSink sink(FLASHSTR("bar.h"), 0, FLASHSTR("Bar!=Baz")); }
  EXPECT_EQ(out_.str(), absl::StrCat(common, "\n"));
}

TEST_F(CheckSinkTest, InsertIntoNonTemporarySink) {
  const std::string_view common = "MCU_CHECK FAILED: SomeExpr ";
  EXPECT_CALL(mock_exit_fn_, Call(common)).Times(1);
  {
    CheckSink sink(nullptr, 0, FLASHSTR("SomeExpr"));
    sink << "abc def";
  }
  EXPECT_EQ(out_.str(), absl::StrCat(common, "abc def\n"));
}

TEST_F(CheckSinkTest, InsertIntoTemporarySink) {
  const std::string_view common =
      "MCU_CHECK FAILED: baz.h:321] expression message ";
  EXPECT_CALL(mock_exit_fn_, Call(common)).Times(1);
  {
    CheckSink sink(MCU_BASENAME("foo/bar/baz.h"), 321,
                   MCU_FLASHSTR("expression message"));
    sink << "abc" << 1.2 << '!';
  }
  EXPECT_EQ(out_.str(), absl::StrCat(common, "abc1.20!\n"));
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
