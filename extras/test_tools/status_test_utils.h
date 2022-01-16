#ifndef MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_

// Insert operator for formatting a Status value and inserting that text into a
// std::ostream.
//
// Author: james.synge@gmail.com

#include <ostream>

#include "extras/test_tools/print_value_to_std_string.h"
#include "status.h"

namespace mcucore {

inline std::ostream& operator<<(std::ostream& out, const Status& status) {
  return out << PrintValueToStdString(status);
}

}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_STATUS_TEST_UTILS_H_
