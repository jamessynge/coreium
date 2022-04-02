#ifndef MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_

// Insert operator for formatting a Status value and inserting that text into a
// std::ostream.
//
// Author: james.synge@gmail.com
//
// PLUS: copied/adapted code from absl/status/status_test.cc
// AND: https://code.tvl.fyi/plain/third_party/nix/src/tests/status_helpers.h

#include <ostream>

#include "extras/test_tools/print_value_to_std_string.h"
#include "gtest/gtest.h"
#include "status.h"

namespace mcucore {

inline std::ostream& operator<<(std::ostream& out, const Status& status) {
  return out << PrintValueToStdString(status);
}

namespace test {

inline const Status& GetStatus(const Status& status) { return status; }

template <typename T>
inline const Status& GetStatus(const StatusOr<T>& status) {
  return status.status();
}

// Monomorphic implementation of matcher IsOkAndHolds(m).  StatusOrType is a
// reference to StatusOr<T>.
template <typename StatusOrType>
class IsOkAndHoldsMatcherImpl
    : public ::testing::MatcherInterface<StatusOrType> {
 public:
  typedef
      typename std::remove_reference<StatusOrType>::type::value_type value_type;

  template <typename InnerMatcher>
  explicit IsOkAndHoldsMatcherImpl(InnerMatcher&& inner_matcher)
      : inner_matcher_(::testing::SafeMatcherCast<const value_type&>(
            std::forward<InnerMatcher>(inner_matcher))) {}

  void DescribeTo(std::ostream* os) const override {
    *os << "is OK and has a value that ";
    inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const override {
    *os << "isn't OK or has a value that ";
    inner_matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(
      StatusOrType actual_value,
      ::testing::MatchResultListener* result_listener) const override {
    if (!actual_value.ok()) {
      *result_listener << "which has status " << actual_value.status();
      return false;
    }

    ::testing::StringMatchResultListener inner_listener;
    const bool matches =
        inner_matcher_.MatchAndExplain(*actual_value, &inner_listener);
    const std::string inner_explanation = inner_listener.str();
    if (!inner_explanation.empty()) {
      *result_listener << "which contains value "
                       << ::testing::PrintToString(*actual_value) << ", "
                       << inner_explanation;
    }
    return matches;
  }

 private:
  const ::testing::Matcher<const value_type&> inner_matcher_;
};

// Implements IsOkAndHolds(m) as a polymorphic matcher.
template <typename InnerMatcher>
class IsOkAndHoldsMatcher {
 public:
  explicit IsOkAndHoldsMatcher(InnerMatcher inner_matcher)
      : inner_matcher_(std::move(inner_matcher)) {}

  // Converts this polymorphic matcher to a monomorphic matcher of the
  // given type.  StatusOrType can be either StatusOr<T> or a
  // reference to StatusOr<T>.
  template <typename StatusOrType>
  operator ::testing::Matcher<StatusOrType>() const {  // NOLINT
    return ::testing::Matcher<StatusOrType>(
        new IsOkAndHoldsMatcherImpl<const StatusOrType&>(inner_matcher_));
  }

 private:
  const InnerMatcher inner_matcher_;
};

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

// Returns a gMock matcher that matches a StatusOr<> whose status is
// OK and whose value matches the inner matcher.
template <typename InnerMatcher>
IsOkAndHoldsMatcher<typename std::decay<InnerMatcher>::type> IsOkAndHolds(
    InnerMatcher&& inner_matcher) {
  return IsOkAndHoldsMatcher<typename std::decay<InnerMatcher>::type>(
      std::forward<InnerMatcher>(inner_matcher));
}

// Returns a gMock matcher that matches a Status or StatusOr<> which is OK.
inline IsOkMatcher IsOk() { return IsOkMatcher(); }

MATCHER_P(StatusCodeIs, code, "") { return arg.code() == code; }

class StatusCodeMatcher {
 public:
  StatusCodeMatcher(StatusCode code) : code_(code) {}

  // Match on Status.
  template <class T,
            typename std::enable_if<std::is_same<T, Status>::value,
                                    int>::type int_ = 0>
  bool MatchAndExplain(const T& status,
                       testing::MatchResultListener* /* listener */) const {
    return status.code() == code_;
  }

  // Match on StatusOr.
  //
  // note: I check for the return value of ConsumeValueOrDie because it's the
  // only non-overloaded member I could figure out how to select. Checking for
  // the presence of .status() didn't work because it's overloaded, so
  // std::invoke_result can't pick which overload to use.
  template <class T,
            typename std::enable_if<
                std::is_same<typename std::invoke_result<
                                 decltype(&T::ConsumeValueOrDie), T>::type,
                             typename T::value_type>::value,
                int>::type int_ = 0>
  bool MatchAndExplain(const T& statusor,
                       MatchResultListener* /* listener */) const {
    return statusor.status().code() == code_;
  }

  void DescribeTo(std::ostream* os) const { *os << "is " << code_; }

  void DescribeNegationTo(std::ostream* os) const { *os << "isn't " << code_; }

 private:
  StatusCode code_;
};


PolymorphicMatcher<StatusCodeMatcher> StatusCodeIs(
    StatusCode code) {
  return testing::MakePolymorphicMatcher(StatusCodeMatcher(code));
}


// TODO(jamessynge): Add a StatusIs matcher.

}  // namespace test
}  // namespace mcucore

// Macros for testing the results of functions that return Status or
// StatusOr<T> (for any type T).
#define ASSERT_OK(expression) ASSERT_THAT(expression, ::mcucore::test::IsOk())
#define EXPECT_OK(expression) EXPECT_THAT(expression, ::mcucore::test::IsOk())

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_
