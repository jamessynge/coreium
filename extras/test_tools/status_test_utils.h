#ifndef MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_

// IsOk() and StatusIs(code, message) supports tests like this:
//
//    EXPECT_THAT(function_returning_status_or_statusor(), IsOk());
//    ASSERT_THAT(function_returning_status_or_statusor(),
//                StatusIs(AnyOf(StatusCode::kUnknown, StatusCode::kInternal),
//                         HasSubstr("some text")));
//
// Also defines macros ASSERT_STATUS_OK and EXPECT_STATUS_OK, to be used like:
//
//    ASSERT_STATUS_OK(function_returning_status_or_statusor());
//    EXPECT_STATUS_OK(some_status_or_statusor_value);
//
// Author: james.synge@gmail.com
// With much help from these sources:
//
// https://github.com/abseil/abseil-cpp/blob/master/absl/status/status_test.cc
// https://github.com/google/crunchy/blob/master/crunchy/internal/common/status_matchers.h
// https://code.tvl.fyi/plain/third_party/nix/src/tests/status_helpers.h

#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include "extras/test_tools/print_value_to_std_string.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "status/status.h"

namespace mcucore {

// Insert a Status into a std::ostream, in support of printing values when an
// EXPECT_* fails.
inline std::ostream& operator<<(std::ostream& out, const Status& status) {
  return out << PrintValueToStdString(status);
}

namespace test {

// Monomorphic implementation of matcher IsOk() for a given type T.
// T can be Status, StatusOr<>, or a reference to either of them.
template <typename T>
class MonoIsOkMatcherImpl : public ::testing::MatcherInterface<T> {
 public:
  void DescribeTo(std::ostream* os) const override { *os << "is OK"; }
  void DescribeNegationTo(std::ostream* os) const override {
    *os << "is not OK";
  }
  bool MatchAndExplain(T actual_value,
                       ::testing::MatchResultListener*) const override {
    return GetStatus(actual_value).ok();
  }
};

// Implements IsOk() as a polymorphic matcher.
class IsOkMatcher {
 public:
  template <typename T>
  operator ::testing::Matcher<T>() const {  // NOLINT
    return ::testing::Matcher<T>(new MonoIsOkMatcherImpl<T>());
  }
};

// Returns a gMock matcher that matches a Status or StatusOr<> which is OK.
inline IsOkMatcher IsOk() { return IsOkMatcher(); }

// Implementation of StatusIs().

// StatusIs() is a polymorphic matcher. This class is the common implementation
// of it shared by all types T where StatusIs() can be used as a Matcher<T>.
class StatusIsMatcherCommonImpl {
 public:
  StatusIsMatcherCommonImpl(
      ::testing::Matcher<StatusCode> code_matcher,
      ::testing::Matcher<const std::string&> message_matcher)
      : code_matcher_(std::move(code_matcher)),
        message_matcher_(std::move(message_matcher)) {}

  void DescribeTo(std::ostream* os) const {
    *os << "has a status code that ";
    code_matcher_.DescribeTo(os);
    *os << " and has an error message that ";
    message_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "has a status code that ";
    code_matcher_.DescribeNegationTo(os);
    *os << " or has an error message that ";
    message_matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(const Status& status,
                       ::testing::MatchResultListener* result_listener) const {
    ::testing::StringMatchResultListener inner_listener;
    if (!code_matcher_.MatchAndExplain(status.code(), &inner_listener)) {
      *result_listener << (inner_listener.str().empty()
                               ? "whose status code is wrong"
                               : "which has a status code " +
                                     inner_listener.str());
      return false;
    }

    if (!message_matcher_.Matches(PrintValueToStdString(status.message()))) {
      *result_listener << "whose error message is wrong";
      return false;
    }

    return true;
  }

 private:
  const ::testing::Matcher<StatusCode> code_matcher_;
  const ::testing::Matcher<const std::string&> message_matcher_;
};

// Monomorphic implementation of matcher StatusIs() for a given type
// T.  T can be Status, StatusOr<>, or a reference to either of them.
template <typename T>
class MonoStatusIsMatcherImpl : public ::testing::MatcherInterface<T> {
 public:
  explicit MonoStatusIsMatcherImpl(StatusIsMatcherCommonImpl common_impl)
      : common_impl_(std::move(common_impl)) {}

  void DescribeTo(std::ostream* os) const override {
    common_impl_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const override {
    common_impl_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(
      T actual_value,
      ::testing::MatchResultListener* result_listener) const override {
    return common_impl_.MatchAndExplain(GetStatus(actual_value),
                                        result_listener);
  }

 private:
  StatusIsMatcherCommonImpl common_impl_;
};

// Implements StatusIs() as a polymorphic matcher.
class StatusIsMatcher {
 public:
  StatusIsMatcher(::testing::Matcher<StatusCode> code_matcher,
                  ::testing::Matcher<const std::string&> message_matcher)
      : common_impl_(std::move(code_matcher), std::move(message_matcher)) {}

  // Converts this polymorphic matcher to a monomorphic matcher of the
  // given type.  T can be StatusOr<>, Status, or a reference to
  // either of them.
  template <typename T>
  operator ::testing::Matcher<T>() const {  // NOLINT
    return ::testing::MakeMatcher(new MonoStatusIsMatcherImpl<T>(common_impl_));
  }

 private:
  const StatusIsMatcherCommonImpl common_impl_;
};

// Returns a gMock matcher that matches a Status or StatusOr<>  whose status
// code matches code_matcher, and whose error message matches message_matcher.
// This depends on the fact that this implicit constructor exists:
//
//     testing::Matcher<T>::Matcher(T value)
//
inline StatusIsMatcher StatusIs(
    ::testing::Matcher<StatusCode> code_matcher,
    ::testing::Matcher<const std::string&> message_matcher) {
  return StatusIsMatcher(std::move(code_matcher), std::move(message_matcher));
}

// Returns a gMock matcher that matches a Status or StatusOr<> whose status code
// matches code_matcher.
inline StatusIsMatcher StatusIs(::testing::Matcher<StatusCode> code_matcher) {
  return StatusIs(std::move(code_matcher), ::testing::_);
}

}  // namespace test
}  // namespace mcucore

// Macros for testing the results of functions that return Status or
// StatusOr<T> (for any type T).
#define ASSERT_STATUS_OK(expression) \
  ASSERT_THAT(expression, ::mcucore::test::IsOk())
#define EXPECT_STATUS_OK(expression) \
  EXPECT_THAT(expression, ::mcucore::test::IsOk())

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_
