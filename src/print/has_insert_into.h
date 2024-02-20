#ifndef MCUCORE_SRC_PRINT_HAS_INSERT_INTO_H_
#define MCUCORE_SRC_PRINT_HAS_INSERT_INTO_H_

// Support for determining at compile time if a value has support for being
// inserted into an OPrintStream. In particular, determining if the value is a
// class or struct instance with an InsertInto(OPrintStream&) method.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "semistd/type_traits.h"

namespace mcucore {

class OPrintStream;  // Forward declaration.

namespace has_insert_into_internal {

// Matches a T that has a InsertInto(OPrintStream&) member function.
template <class T>
static auto test_insert_into(int)
    -> sfinae_true<
        decltype(declval<T>().InsertInto(declval<::mcucore::OPrintStream&>()))>;

// SFINAE fallback for the case where T does not have an InsertInto member
// function with the required signature. This depends on the fact that type of
// the literal '0' is int, so the compiler will prefer a match to the prior
// function, but will fallback to this one instead of emitting an error.
template <typename T>
static auto test_insert_into(long) -> false_type;  // NOLINT

}  // namespace has_insert_into_internal

// has_insert_into extends either true_type or false_type, depending on whether
// T has an InsertInto(OPrintStream&) member function.
template <typename T>
struct has_insert_into
    : decltype(has_insert_into_internal::test_insert_into<T>(0)) {};

}  // namespace mcucore

#endif  // MCUCORE_SRC_PRINT_HAS_INSERT_INTO_H_
