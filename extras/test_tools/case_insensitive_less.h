#ifndef ARDUINO_MCUCORE_EXTRAS_TEST_TOOLS_CASE_INSENSITIVE_LESS_H_
#define ARDUINO_MCUCORE_EXTRAS_TEST_TOOLS_CASE_INSENSITIVE_LESS_H_

#include <string>

namespace mcucore {
namespace test {

struct CaseInsensitiveLess {
  bool operator()(const std::string& lhs, const std::string& rhs) const;
};

}  // namespace test
}  // namespace mcucore

#endif  // ARDUINO_MCUCORE_EXTRAS_TEST_TOOLS_CASE_INSENSITIVE_LESS_H_
