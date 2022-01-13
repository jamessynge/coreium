#include "log_sink.h"

#include "extras/test_tools/print_to_std_string.h"
#include "gtest/gtest.h"
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

  // The EXPECT_DEATH created a subprocess, so out shouldn't have been modified
  // in this process.
  EXPECT_EQ(out.str(), "");
}

TEST(CheckSinkDeathTest, NoFileName) {
  EXPECT_DEATH({ CheckSink sink(nullptr, 123, MCU_FLASHSTR("prefix2")); },
               "MCU_CHECK FAILED: prefix2");
}

TEST(CheckSinkDeathTest, NoLineNumber) {
  mcucore::test::PrintToStdString out;
  EXPECT_DEATH(
      {
        CheckSink sink(out, MCU_FLASHSTR("foo.cc"), 0, MCU_FLASHSTR("prefix3"));
      },
      "MCU_CHECK FAILED: foo.cc] prefix3");

  // The EXPECT_DEATH created a subprocess, so out shouldn't have been modified
  // in this process.
  EXPECT_EQ(out.str(), "");
}

TEST(CheckSinkDeathTest, InsertIntoNonTemporarySink) {
  EXPECT_DEATH(
      {
        CheckSink sink(MCU_FLASHSTR_32("foo/bar/baz.cc"), 234,
                       MCU_FLASHSTR_32("Prefix4"));
        sink << "abc def";
      },
      "MCU_CHECK FAILED: foo/bar/baz.cc:234] Prefix4 abc def");
}

TEST(CheckSinkDeathTest, InsertIntoTemporarySink) {
  EXPECT_DEATH(
      {
        CheckSink(MCU_BASENAME("foo/bar/baz.h"), 321, MCU_FLASHSTR("message"))
            << "abc";
      },
      "MCU_CHECK FAILED: baz.h:321] message abc");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
