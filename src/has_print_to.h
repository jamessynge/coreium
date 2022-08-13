#ifndef MCUCORE_SRC_HAS_PRINT_TO_H_
#define MCUCORE_SRC_HAS_PRINT_TO_H_

// Support for determining at compile time if a value of type T has support for
// being printed. In particular, determining if:
//
// A) T is a class or struct with a printTo method that has a signature
//    compatible with Arduino's Printable::printTo method;
//
// B) there exists a PrintValueTo(Print& out, const T& value) function, which is
//    presumed to print the value to the Print instance.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "semistd/type_traits.h"

namespace mcucore {
namespace has_print_to_internal {

// Matches a T that has a printTo(Print&) member function.
template <class T>
static auto test_print_to(int)
    -> sfinae_true<decltype(declval<T>().printTo(declval<::Print&>()))>;

// SFINAE fallback for the case where T does not have a printTo(Print&) member
// function. This depends on the fact that type of the literal '0' is int, so
// the compiler will prefer a match to the prior function, but will fallback to
// this one instead of emitting an error.
template <typename T>
static auto test_print_to(long) -> false_type;  // NOLINT

}  // namespace has_print_to_internal

// has_print_to extends either true_type or false_type, depending on whether T
// has a printTo(Print&) member function.
template <typename T>
struct has_print_to : decltype(has_print_to_internal::test_print_to<T>(0)) {};

// has_print_value_to extends either true_type or false_type, depending on
// whether there exists a PrintValueTo(T, Print&) function. The first definition
// matches any type for which there is not a corresponding PrintValueTo
// function.
template <class, class = void>
struct has_print_value_to : false_type {};

// Matches a T for which there is a PrintValueTo(T, Print&) function.
template <class T>
struct has_print_value_to<
    T, void_t<decltype(PrintValueTo(declval<T>(), declval<::Print&>()))>>
    : true_type {};

}  // namespace mcucore

#endif  // MCUCORE_SRC_HAS_PRINT_TO_H_
