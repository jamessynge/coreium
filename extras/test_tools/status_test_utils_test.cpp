#include "extras/test_tools/status_test_utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "status/status.h"
#include "strings/progmem_string_view.h"
#include "testing/base/public/gunit-spi.h"

namespace mcucore {
namespace test {
namespace {

using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;

// A helper to enable checking for indirection (i.e. where a function call is
// supplying the value, rather than a value being provided explicitly).
class OkStatusSource {
 public:
  const Status& status() const { return status_; }

 private:
  const Status status_{OkStatus()};
};

// A helper to enable checking for indirection (i.e. where a function call is
// supplying the value, rather than a value being provided explicitly).
class SomeStatusSource {
 public:
  explicit SomeStatusSource(const Status& status) : some_status_(status) {}

  const Status& status() const { return some_status_; }

 private:
  const Status some_status_;
};

TEST(StatusTestUtilsTest, StdOStream) {
  {
    std::ostringstream oss;
    oss << OkStatus();
    EXPECT_THAT(oss.str(), Eq("OK"));
  }
  {
    std::ostringstream oss;
    oss << UnknownError();
    EXPECT_THAT(oss.str(), HasSubstr("{.code=Unknown}"));
  }
  {
    std::ostringstream oss;
    oss << InternalError(MCU_PSV("Unknown"));
    EXPECT_THAT(oss.str(),
                HasSubstr(R"({.code=Internal, .message="Unknown"})"));
  }
}

TEST(StatusTestUtilsTest, IsOkDescriptions) {
  auto is_ok_matcher = IsOk();
  testing::Matcher<Status> matcher = is_ok_matcher;

  {
    std::ostringstream oss;
    matcher.DescribeTo(&oss);
    EXPECT_THAT(oss.str(), Eq("is OK"));
  }
  {
    std::ostringstream oss;
    matcher.DescribeNegationTo(&oss);
    EXPECT_THAT(oss.str(), Eq("is not OK"));
  }
}

TEST(StatusTestUtilsTest, IsOk) {
  EXPECT_THAT(OkStatus(), IsOk());
  EXPECT_THAT(OkStatusSource(), IsOk());

  {
    Status status = OkStatus();
    EXPECT_THAT(status, IsOk());
    auto fn = [](const Status& status) { EXPECT_THAT(status, IsOk()); };
    fn(status);
  }
}

TEST(StatusTestUtilsTest, NotIsOk) {
  EXPECT_THAT(UnknownError(MCU_PSV("OK")), Not(IsOk()));
  EXPECT_THAT(SomeStatusSource(DataLossError(MCU_PSV("Meh"))), Not(IsOk()));

  {
    Status status = UnknownError();
    EXPECT_THAT(status, Not(IsOk()));
    auto fn = [](const Status& status) { EXPECT_THAT(status, Not(IsOk())); };
    fn(status);
  }
}

TEST(StatusTestUtilsTest, StatusIsCodeDescriptions) {
  auto status_code_is_matcher = StatusIs(StatusCode::kFailedPrecondition);
  testing::Matcher<Status> matcher = status_code_is_matcher;

  {
    std::ostringstream oss;
    matcher.DescribeTo(&oss);
    EXPECT_THAT(oss.str(), AllOf(HasSubstr("has a status code that"),
                                 HasSubstr(" is equal "),
                                 HasSubstr(" FailedPrecondition "),
                                 HasSubstr(" and has an error message that "),
                                 HasSubstr("is anything")));
  }
  {
    std::ostringstream oss;
    matcher.DescribeNegationTo(&oss);
    EXPECT_THAT(oss.str(), AllOf(HasSubstr("has a status code that"),
                                 HasSubstr(" isn't equal "),
                                 HasSubstr(" FailedPrecondition "),
                                 HasSubstr(" or has an error message that "),
                                 HasSubstr("never matches")));
  }
}

TEST(StatusTestUtilsTest, StatusIsCode) {
  EXPECT_THAT(OkStatus(), StatusIs(StatusCode::kOk));
  EXPECT_THAT(OkStatusSource(), StatusIs(StatusCode::kOk));
  EXPECT_THAT(UnknownError(), StatusIs(StatusCode::kUnknown));
  EXPECT_THAT(SomeStatusSource(ResourceExhaustedError(MCU_PSV("All gone!"))),
              StatusIs(StatusCode::kResourceExhausted));
}

TEST(StatusTestUtilsTest, NotStatusIsCode) {
  EXPECT_THAT(OkStatus(), Not(StatusIs(StatusCode::kUnknown)));
  EXPECT_THAT(OkStatusSource(), Not(StatusIs(StatusCode::kUnknown)));
  EXPECT_THAT(UnknownError(), Not(StatusIs(StatusCode::kOk)));
  EXPECT_THAT(SomeStatusSource(ResourceExhaustedError()),
              Not(StatusIs(StatusCode::kOk)));
}

TEST(StatusTestUtilsTest, StatusIsCodeMatcher) {
  EXPECT_THAT(OkStatus(), StatusIs(AnyOf(StatusCode::kOk, StatusCode::kOk)));
  EXPECT_THAT(OkStatusSource(), StatusIs(StatusCode::kOk));
  EXPECT_THAT(UnknownError(), StatusIs(StatusCode::kUnknown));
  EXPECT_THAT(SomeStatusSource(ResourceExhaustedError(MCU_PSV("All gone!"))),
              StatusIs(StatusCode::kResourceExhausted));

  EXPECT_THAT(OkStatus(), Not(StatusIs(StatusCode::kUnknown)));
  EXPECT_THAT(OkStatusSource(), Not(StatusIs(StatusCode::kUnknown)));
  EXPECT_THAT(UnknownError(), Not(StatusIs(StatusCode::kOk)));
  EXPECT_THAT(SomeStatusSource(ResourceExhaustedError()),
              Not(StatusIs(StatusCode::kOk)));
}

TEST(StatusTestUtilsTest, StatusIsCodeAndStringDescriptions) {
  auto status_code_is_matcher =
      StatusIs(StatusCode::kInternal, HasSubstr("FooBar"));
  testing::Matcher<Status> matcher = status_code_is_matcher;

  {
    std::ostringstream oss;
    matcher.DescribeTo(&oss);
    EXPECT_THAT(
        oss.str(),
        AllOf(HasSubstr("has a status code that"), HasSubstr(" Internal "),
              HasSubstr(" and has an error message that "),
              HasSubstr(" has substring "), HasSubstr("\"FooBar\"")));
  }
  {
    std::ostringstream oss;
    matcher.DescribeNegationTo(&oss);
    EXPECT_THAT(
        oss.str(),
        AllOf(HasSubstr("has a status code that"), HasSubstr(" isn't equal "),
              HasSubstr(" Internal "),
              HasSubstr(" or has an error message that "),
              HasSubstr(" has no substring "), HasSubstr("\"FooBar\"")));
  }
}

TEST(StatusTestUtilsTest, StatusIsCodeAndString) {
  EXPECT_THAT(OkStatus(), StatusIs(StatusCode::kOk, ""));
  EXPECT_THAT(OkStatus(), StatusIs(StatusCode::kOk, HasSubstr("")));

  EXPECT_THAT(UnknownError(MCU_PSV("Xanadu")),
              StatusIs(StatusCode::kUnknown, HasSubstr("anad")));
  EXPECT_THAT(UnknownError(MCU_PSV("Xanadu")),
              StatusIs(StatusCode::kUnknown, Eq("Xanadu")));
  EXPECT_THAT(UnknownError(MCU_PSV("Xanadu")),
              StatusIs(StatusCode::kUnknown, "Xanadu"));

  EXPECT_THAT(SomeStatusSource(ResourceExhaustedError(MCU_PSV("All gone!"))),
              StatusIs(StatusCode::kResourceExhausted, "All gone!"));
}

TEST(StatusTestUtilsTest, NotStatusIsCodeAndString) {
  // OkStatus doesn't match both code and message.
  EXPECT_THAT(OkStatus(), Not(StatusIs(StatusCode::kUnknown, HasSubstr(""))));
  EXPECT_THAT(OkStatusSource(), Not(StatusIs(StatusCode::kOk, Eq("OK!"))));

  // Status doesn't match both code and string.
  EXPECT_THAT(UnknownError(), Not(StatusIs(StatusCode::kOk, HasSubstr(""))));
  EXPECT_THAT(UnknownError(MCU_PSV("FooBar")),
              Not(StatusIs(StatusCode::kUnknown, HasSubstr("Unknown"))));
  EXPECT_THAT(SomeStatusSource(InternalError(MCU_PSV("So tired"))),
              Not(StatusIs(StatusCode::kResourceExhausted, "So tired")));
}

TEST(StatusTestUtilsTest, ExpectStatusOk) {
  EXPECT_STATUS_OK(OkStatus());
  EXPECT_STATUS_OK(OkStatusSource());
  EXPECT_NONFATAL_FAILURE(EXPECT_STATUS_OK(UnknownError()), "code=Unknown");
}

TEST(StatusTestUtilsTest, AssertStatusOk) {
  ASSERT_STATUS_OK(OkStatus());
  ASSERT_STATUS_OK(OkStatusSource());
  EXPECT_FATAL_FAILURE(ASSERT_STATUS_OK(DataLossError(MCU_PSV("Xanadu"))),
                       R"(code=DataLoss, .message="Xanadu")");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
