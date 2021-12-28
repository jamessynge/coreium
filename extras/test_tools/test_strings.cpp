#include "extras/test_tools/test_strings.h"

#include <string>
#include <string_view>

namespace mcucore {
namespace test {

std::string GenerateTestString(const size_t length) {
  std::string s;
  s.reserve(length);
  const std::string_view alphas =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  size_t alpha_ndx = 0;
  for (size_t ndx = 1; ndx <= length; ++ndx) {
    if (ndx % 100 == 0) {
      s.push_back('.');
    } else if (ndx % 10 == 0) {
      s.push_back(alphas[alpha_ndx++ % alphas.size()]);
    } else {
      s.push_back('0' + (ndx % 10));
    }
  }
  return s;
}

}  // namespace test
}  // namespace mcucore
