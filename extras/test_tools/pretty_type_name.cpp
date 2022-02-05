#include "extras/test_tools/pretty_type_name.h"

namespace mcucore {
namespace test {
namespace pretty_internal {
static constexpr auto kSentinalName = PrettyFunctionName<double>();
static constexpr auto kPrefixSize = kSentinalName.find("double");
static constexpr auto kSuffixSize =
    kSentinalName.size() - kSentinalName.rfind("double") - 6;

std::string_view TrimToTypeName(std::string_view pretty_function_name) {
  auto without_prefix = pretty_function_name.substr(kPrefixSize);
  return without_prefix.substr(0, without_prefix.size() - kSuffixSize);
}

}  // namespace pretty_internal
}  // namespace test
}  // namespace mcucore
