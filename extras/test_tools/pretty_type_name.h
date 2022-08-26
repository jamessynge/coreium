#ifndef MCUCORE_EXTRAS_TEST_TOOLS_PRETTY_TYPE_NAME_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_PRETTY_TYPE_NAME_H_

// PrettyTypeName<typename T> returns T as a string T, i.e. PrettyTypeName<int>
// returns "int".
//
// Author: james.synge@gmail.com

#include <string>
#include <string_view>

namespace mcucore {
namespace test {
namespace pretty_internal {

// Returns the name of the function, e.g. "PrettyFunctionName<int>", or
// "PrettyFunctionName<T>, with T = int".
template <typename T>
constexpr std::string_view PrettyFunctionName() {
#if defined(__clang__) || defined(__GNUC__)
  return std::string_view{__PRETTY_FUNCTION__};
#elif defined(__MSC_VER)
  return std::string_view{__FUNCSIG__};
#else
#error Unsupported compiler
#endif
}

// Returns the name of the type T from the string returned by
// PrettyFunctionName<T>().
std::string_view TrimToTypeName(std::string_view pretty_function_name);

}  // namespace pretty_internal

template <typename T>
std::string_view PrettyTypeName() {
  return pretty_internal::TrimToTypeName(
      pretty_internal::PrettyFunctionName<T>());
}

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_PRETTY_TYPE_NAME_H_
