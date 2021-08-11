#ifndef MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_VIEW_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_VIEW_UTILS_H_

// Host utilities for working with mcucore::ProgmemStringView in the context of
// the C++ standard library.
//
// Author: james.synge@gmail.com

#include <ostream>

#include "progmem_string_view.h"

namespace mcucore {

inline std::ostream& operator<<(std::ostream& out,
                                const ProgmemStringView& view) {
  for (const char c : view) {
    out << c;
  }
  return out;
}

}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_VIEW_UTILS_H_
