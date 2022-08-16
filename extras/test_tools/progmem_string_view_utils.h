#ifndef MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_VIEW_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_VIEW_UTILS_H_

// Host utilities for working with mcucore::ProgmemStringView in the context of
// the C++ standard library.
//
// Author: james.synge@gmail.com

#include <ostream>
#include <string>
#include <string_view>

#include "extras/test_tools/print_value_to_std_string.h"
#include "strings/progmem_string_view.h"

namespace mcucore {
namespace test {

inline ProgmemStringView MakeProgmemStringView(const std::string& str) {
  return ProgmemStringView(str.data(), str.size());
}

inline ProgmemStringView MakeProgmemStringView(std::string_view view) {
  return ProgmemStringView(view.data(), view.size());
}

inline std::string MakeStdString(const ProgmemStringView& view) {
  return PrintValueToStdString(view);
}

}  // namespace test

inline std::ostream& operator<<(std::ostream& out,
                                const ProgmemStringView& view) {
  for (const char c : view) {
    out << c;
  }
  return out;
}

}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_VIEW_UTILS_H_
