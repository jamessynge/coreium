#include "extras/test_tools/status_or_test_utils.h"

#include <sstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "status/status.h"
#include "status/status_or.h"
#include "testing/base/public/gunit-spi.h"

namespace mcucore {
namespace test {
namespace {

using ::testing::AllOf;
using ::testing::EndsWith;
using ::testing::Eq;
using ::testing::Not;
using ::testing::StartsWith;

// A helper to enable checking for indirection (i.e. where a function call is
// supplying the value, rather than a value being provided explicitly).
class StatusOrIntSource {
 public:
  explicit StatusOrIntSource(int value) : status_or_(value) {}
  explicit StatusOrIntSource(const Status& status) : status_or_(status) {}

  const StatusOr<int>& status_or() const { return status_or_; }

 private:
  const StatusOr<int> status_or_;
};

TEST(StatusOrTestUtilsTest, OpInsert) {
  {
    std::ostringstream oss;
    oss << StatusOr<int>(UnknownError());
    EXPECT_EQ(oss.str(), "{.code=Unknown}");
  }
  {
    std::ostringstream oss;
    oss << StatusOr<int>(1);
    EXPECT_EQ(oss.str(), "{.value=1}");
  }
}

TEST(StatusOrTestUtilsTest, IsOkAndHolds) {
  EXPECT_THAT(StatusOr<int>(7), IsOkAndHolds(7));
  EXPECT_THAT(StatusOr<int>(6), IsOkAndHolds(Not(7)));
}

TEST(StatusOrTestUtilsTest, NotIsOkAndHolds) {
  EXPECT_THAT(StatusOr<int>(6), Not(IsOkAndHolds(7)));
  EXPECT_THAT(StatusOr<int>(UnknownError()), Not(IsOkAndHolds(7)));
}

TEST(StatusOrTestUtilsTest, IsOkAndHoldsFailures) {
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(StatusOr<int>(6), IsOkAndHolds(7)),
                          "Expected: is OK and has a value that");
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(StatusOr<int>(6), IsOkAndHolds(Eq(7))),
                          "Expected: is OK and has a value that");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(StatusOr<int>(6), IsOkAndHolds(Not(Eq(6)))),
      "Expected: is OK and has a value that");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(StatusOr<int>(ResourceExhaustedError()), IsOkAndHolds(7)),
      "which has status {.code=ResourceExhausted}");
}

TEST(StatusOrTestUtilsTest, IsOkAndHoldsDescriptions) {
  auto is_ok_and_holds_matcher = IsOkAndHolds(0);
  testing::Matcher<StatusOr<int>> matcher = is_ok_and_holds_matcher;

  {
    std::ostringstream oss;
    matcher.DescribeTo(&oss);
    EXPECT_THAT(oss.str(), AllOf(StartsWith("is OK and has a value that"),
                                 EndsWith(" 0")));
  }

  {
    std::ostringstream oss;
    matcher.DescribeNegationTo(&oss);
    EXPECT_THAT(oss.str(), AllOf(StartsWith("isn't OK or has a value that"),
                                 EndsWith(" 0")));
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
