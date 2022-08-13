#ifndef MCUCORE_SRC_SEMISTD_UTILITY_H_
#define MCUCORE_SRC_SEMISTD_UTILITY_H_

// Implementations of selected C++ <utility> library features, though defined in
// the mcucore namespace, rather than std.

#include "mcucore_platform.h"
#include "semistd/type_traits.h"

namespace mcucore {

// Forwards lvalues as either lvalues or as rvalues, depending on T.
template <class T>
constexpr T&& forward(typename remove_reference<T>::type& t) {
  return static_cast<T&&>(t);
}

// Forwards rvalues as rvalues and prohibits forwarding of rvalues as lvalues.
template <class T>
constexpr T&& forward(typename remove_reference<T>::type&& t) {
  return static_cast<T&&>(t);
}

template <class T>
constexpr remove_reference_t<T>&& move(T&& t) {
  return static_cast<remove_reference_t<T>&&>(t);
}

}  // namespace mcucore

#endif  // MCUCORE_SRC_SEMISTD_UTILITY_H_
