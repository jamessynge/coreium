#include "log_sink.h"

#include "extras/test_tools/print_to_std_string.h"
#include "gtest/gtest.h"
#include "inline_literal.h"
#include "o_print_stream.h"
#include "progmem_string_data.h"

namespace mcucore {
namespace test {
namespace {

TEST(LogSinkTest, CreateAndDelete) {
  mcucore::test::PrintToStdString out;
  { LogSink sink(out); }
  // Prints out a newline to end the message. So far there isn't a message
  // prefix (i.e. no time or location).
  EXPECT_EQ(out.str(), "\n");
}

TEST(LogSinkTest, InsertIntoNonTemporary) {
  mcucore::test::PrintToStdString out;
  {
    LogSink sink(out);
    sink << "abc";
  }
  EXPECT_EQ(out.str(), "abc\n");
}

TEST(LogSinkTest, InsertIntoTemporary) {
  mcucore::test::PrintToStdString out;
  LogSink(out) << "abc";
  EXPECT_EQ(out.str(), "abc\n");
}

TEST(CheckSinkDeathTest, CreateAndDelete) {
  mcucore::test::PrintToStdString out;
  EXPECT_DEATH(
      {
        CheckSink sink(out, MCU_FLASHSTR("foo.cc"), 123,
                       MCU_FLASHSTR("prefix1"));
      },
      "MCU_CHECK FAILED: foo.cc:123] prefix1");
}

TEST(CheckSinkDeathTest, NoFileName) {
  mcucore::test::PrintToStdString out;
  EXPECT_DEATH({ CheckSink sink(out, nullptr, 123, MCU_FLASHSTR("prefix2")); },
               "MCU_CHECK FAILED: prefix2");
}

TEST(CheckSinkDeathTest, NoLineNumber) {
  mcucore::test::PrintToStdString out;
  EXPECT_DEATH(
      {
        CheckSink sink(out, MCU_FLASHSTR("foo.cc"), 0, MCU_FLASHSTR("prefix3"));
      },
      "MCU_CHECK FAILED: foo.cc] prefix3");
}

TEST(CheckSinkDeathTest, InsertIntoNonTemporary) {
  mcucore::test::PrintToStdString out;
  EXPECT_DEATH(
      {
        CheckSink sink(out, MCU_FLASHSTR("foo/bar.cc"), 234,
                       MCU_FLASHSTR("Prefix4"));
        sink << "abc";
      },
      "MCU_CHECK FAILED: bar.cc:234] Prefix4");
}

TEST(CheckSinkDeathTest, InsertIntoTemporary) {
  mcucore::test::PrintToStdString out;
  EXPECT_DEATH(
      {
        CheckSink(out, MCU_FLASHSTR("foo/bar/baz.h"), 321,
                  MCU_FLASHSTR("message"))
            << "abc";
      },
      "MCU_CHECK FAILED: baz.h:321] message");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
