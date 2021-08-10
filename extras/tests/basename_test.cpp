#include "basename.h"

#include "extras/test_tools/print_to_std_string.h"
#include "gtest/gtest.h"
#include "inline_literal.h"

namespace mcucore {

namespace test {
namespace {

TEST(TasBasenameTest, NoSlash) {
  mcucore::test::PrintToStdString out;
  out.print(TAS_BASENAME("foo.bar.baz"));
  EXPECT_EQ(out.str(), "foo.bar.baz");
}

TEST(TasBasenameTest, LeadingSlash) {
  mcucore::test::PrintToStdString out;
  out.print(TAS_BASENAME("/bar.baz"));
  EXPECT_EQ(out.str(), "bar.baz");
}

TEST(TasBasenameTest, LeadingSlashes) {
  mcucore::test::PrintToStdString out;
  out.print(TAS_BASENAME("//bar.baz"));
  EXPECT_EQ(out.str(), "bar.baz");
}

TEST(TasBasenameTest, MiddleSlash) {
  mcucore::test::PrintToStdString out;
  out.print(TAS_BASENAME("foo/bar.baz"));
  EXPECT_EQ(out.str(), "bar.baz");
}

TEST(TasBasenameTest, LeadingAndMiddleSlashes) {
  mcucore::test::PrintToStdString out;
  out.print(TAS_BASENAME("//foo//bar/baz.cc"));
  EXPECT_EQ(out.str(), "baz.cc");
}

TEST(TasBasenameTest, TrailingSlash) {
  mcucore::test::PrintToStdString out;
  out.print(TAS_BASENAME("foo.bar.baz/"));
  EXPECT_EQ(out.str(), "");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
