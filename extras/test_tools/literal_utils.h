#ifndef MCUCORE_EXTRAS_TEST_TOOLS_LITERAL_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_LITERAL_UTILS_H_

// Host utilities for working with mcucore::Literal in the context of the C++
// standard library.
//
// Author: james.synge@gmail.com

#include <ostream>

#include "literal.h"

namespace mcucore {

inline std::ostream& operator<<(std::ostream& out, const Literal& literal) {
  for (const char c : literal) {
    out << c;
  }
  return out;
}

}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_LITERAL_UTILS_H_
