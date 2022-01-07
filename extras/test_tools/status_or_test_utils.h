#ifndef MCUCORE_EXTRAS_TEST_TOOLS_STATUS_OR_TEST_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_STATUS_OR_TEST_UTILS_H_

#include <ostream>

#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/status_test_utils.h"
#include "status_or.h"

namespace mcucore {

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
