#ifndef MCUCORE_EXTRAS_TEST_TOOLS_STATUS_OR_TEST_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_STATUS_OR_TEST_UTILS_H_

// Insert operator for formatting a StatusOr<T> value and inserting that text
// into a std::ostream, and operator== in support of EXPECT_EQ.
//
// Author: james.synge@gmail.com

#include <ostream>

#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/status_test_utils.h"
#include "status_or.h"

namespace mcucore {

template <typename T>
bool operator==(const StatusOr<T>& a, const StatusOr<T>& b) {
  if (a.ok()) {
    return b.ok() && a.value() == b.value();
  } else {
    return a.status() == b.status();
  }
}

template <typename T>
bool operator==(const StatusOr<T>& a, const Status& b) {
  return !a.ok() && a.status() == b;
}

template <typename T>
bool operator==(const Status& a, const StatusOr<T>& b) {
  return !b.ok() && b.status() == a;
}

template <typename T>
bool operator==(const StatusOr<T>& a, const T& b) {
  return a.ok() && a.value() == b;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const StatusOr<T>& status_or) {
  if (status_or.ok()) {
    return out << "{.value=" << PrintValueToStdString(status_or.value()) << "}";
  } else {
    return out << status_or.status();
  }
}

}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_STATUS_OR_TEST_UTILS_H_
