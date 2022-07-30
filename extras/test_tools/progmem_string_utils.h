#ifndef MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_UTILS_H_

// Host utilities for working with mcucore::ProgmemString in the context of the
// C++ standard library and googletest.
//
// Author: james.synge@gmail.com

#include <ostream>
#include <string>
#include <string_view>

#include "extras/test_tools/print_value_to_std_string.h"
#include "progmem_string.h"

namespace mcucore {
namespace test {

inline ProgmemString MakeProgmemStringView(const std::string& str) {
  return ProgmemString(
      reinterpret_cast<const __FlashStringHelper*>((str.c_str())));
}

inline ProgmemString MakeProgmemStringView(const char* str) {
  return ProgmemString(reinterpret_cast<const __FlashStringHelper*>((str)));
}

inline std::string MakeStdString(const ProgmemString& str) {
  return PrintValueToStdString(str);
}

}  // namespace test

inline std::ostream& operator<<(std::ostream& out, const ProgmemString& str) {
  return out << PrintValueToStdString(str);
}

inline bool operator==(const ProgmemString& str1, std::string_view str2) {
  return PrintValueToStdString(str1) == PrintValueToStdString(str2);
}

}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_PROGMEM_STRING_UTILS_H_
