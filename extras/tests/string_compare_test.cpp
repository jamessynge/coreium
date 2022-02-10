#include "string_compare.h"

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/sample_printable.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "progmem_string_view.h"
#include "string_view.h"

namespace mcucore {
namespace test {
namespace {

TEST(StringCompareTest, EqualValues) {
  ProgmemStringView psv("abc");
  StringView sv("abc");

  // ProgmemStringView as LHS/first arg.
  EXPECT_TRUE(psv == sv);
  EXPECT_EQ(psv, sv);
  EXPECT_TRUE(ExactlyEqual(psv, sv));
  EXPECT_TRUE(CaseEqual(psv, sv));
  EXPECT_TRUE(LoweredEqual(psv, sv));

  // StringView as LHS/first arg.
  EXPECT_TRUE(sv == psv);
  EXPECT_EQ(sv, psv);
  EXPECT_TRUE(CaseEqual(sv, psv));
  EXPECT_TRUE(StartsWith(sv, psv));
}

TEST(StringCompareTest, NotEqualValues) {
  {
    // Same length.
    ProgmemStringView psv("abc11");
    StringView sv("abc21");

    // ProgmemStringView as LHS/first arg.
    EXPECT_FALSE(psv == sv);
    EXPECT_NE(psv, sv);
    EXPECT_FALSE(ExactlyEqual(psv, sv));
    EXPECT_FALSE(CaseEqual(psv, sv));
    EXPECT_FALSE(LoweredEqual(psv, sv));

    // StringView as LHS/first arg.
    EXPECT_FALSE(sv == psv);
    EXPECT_NE(sv, psv);
    EXPECT_FALSE(CaseEqual(sv, psv));
    EXPECT_FALSE(StartsWith(sv, psv));
  }
  {
    // Different length.
    ProgmemStringView psv("abc1");
    StringView sv("abc12");

    // ProgmemStringView as LHS/first arg.
    EXPECT_FALSE(psv == sv);
    EXPECT_NE(psv, sv);
    EXPECT_FALSE(ExactlyEqual(psv, sv));
    EXPECT_FALSE(CaseEqual(psv, sv));
    EXPECT_FALSE(LoweredEqual(psv, sv));

    // StringView as LHS/first arg.
    EXPECT_FALSE(sv == psv);
    EXPECT_NE(sv, psv);
    EXPECT_FALSE(CaseEqual(sv, psv));
    EXPECT_TRUE(StartsWith(sv, psv));
  }
  {
    // Different length.
    ProgmemStringView psv("abc1");
    StringView sv("abc21");

    // ProgmemStringView as LHS/first arg.
    EXPECT_FALSE(psv == sv);
    EXPECT_NE(psv, sv);
    EXPECT_FALSE(ExactlyEqual(psv, sv));
    EXPECT_FALSE(CaseEqual(psv, sv));
    EXPECT_FALSE(LoweredEqual(psv, sv));

    // StringView as LHS/first arg.
    EXPECT_FALSE(sv == psv);
    EXPECT_NE(sv, psv);
    EXPECT_FALSE(CaseEqual(sv, psv));
    EXPECT_FALSE(StartsWith(sv, psv));
  }
}

TEST(StringCompareTest, StartsWith) {
  {
    ProgmemStringView psv("prefix");
    StringView sv("prefix suffix");
    EXPECT_TRUE(StartsWith(sv, psv));
  }
  {
    ProgmemStringView psv("not prefix");
    StringView sv("prefix suffix");
    EXPECT_FALSE(StartsWith(sv, psv));
  }
  {
    ProgmemStringView psv("suffix");
    StringView sv("prefix suffix");
    EXPECT_FALSE(StartsWith(sv, psv));
  }
  {
    ProgmemStringView psv("too long to be a prefix");
    StringView sv("short");
    EXPECT_FALSE(StartsWith(sv, psv));
  }
}

TEST(StringCompareTest, LoweredEqual) {
  {
    ProgmemStringView psv("all equal");
    StringView sv("all equal");
    EXPECT_TRUE(LoweredEqual(psv, sv));
  }
  {
    StringView sv("all lower case");
    ProgmemStringView psv("All Lower Case");
    EXPECT_TRUE(LoweredEqual(psv, sv));
  }
  {
    StringView sv("All Lower Case");
    ProgmemStringView psv("all lower case");
    EXPECT_FALSE(LoweredEqual(psv, sv));
  }
  {
    StringView sv("not equal");
    ProgmemStringView psv("not the same");
    EXPECT_FALSE(LoweredEqual(psv, sv));
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
