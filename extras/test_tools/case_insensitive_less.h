#ifndef MCUCORE_EXTRAS_TEST_TOOLS_CASE_INSENSITIVE_LESS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_CASE_INSENSITIVE_LESS_H_

// CaseInsensitiveLess is a function object for comparing two strings in a
// case-insensitive manner. It can be used where std::less is used for computing
// an order for strings.
//
// Author: james.synge@gmail.com

#include <string>

namespace mcucore {
namespace test {

struct CaseInsensitiveLess {
  bool operator()(const std::string& lhs, const std::string& rhs) const;
};

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_CASE_INSENSITIVE_LESS_H_
