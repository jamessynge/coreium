#ifndef MCUCORE_SRC_HAS_PROGMEM_CHAR_ARRAY_H_
#define MCUCORE_SRC_HAS_PROGMEM_CHAR_ARRAY_H_

// Support for determining at compile time if a type has a method returning a
// a PROGMEM string. In particular, if the type has a method called
// `progmem_char_array()`.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"  // IWYU pragma: keep
#include "type_traits.h"

namespace mcucore {

namespace has_progmem_char_array_internal {

// Matches a T that has a member function matching:
//
//       const char(&)[N] progmem_char_array()
//
// Note that I haven't (yet) figured out a way to validate that the return type
// of the method matches.
template <class T>
static auto test_progmem_array(int)
    -> sfinae_true<decltype(declval<T>().progmem_char_array())>;

// SFINAE fallback for the case where T does not have a progmem_char_array()
// member function. This depends on the fact that type of the literal '0' is
// int, so the compiler will prefer a match to the prior function, but will
// fallback to this one instead of emitting an error.
template <typename T>
static auto test_progmem_array(long) -> false_type;  // NOLINT

}  // namespace has_progmem_char_array_internal

// progmem_char_array extends either true_type or false_type, depending on
// whether T has a progmem_char_array() member function.
template <typename T>
struct has_progmem_char_array
    : decltype(has_progmem_char_array_internal::test_progmem_array<
               remove_cv_t<T> >(0)) {};

// Since a common use case for has_progmem_char_array is to limit application
// of a template function to types with a progmem_char_array, this helper
// combines enable_if and has_progmem_char_array.
template <typename T>
struct enable_if_has_progmem_char_array
    : enable_if_t<has_progmem_char_array<T>::value> {};

template <typename T>
using enable_if_has_progmem_char_array_t =
    typename enable_if_has_progmem_char_array<T>::type;

}  // namespace mcucore

#endif  // MCUCORE_SRC_HAS_PROGMEM_CHAR_ARRAY_H_
