#ifndef MCUCORE_SRC_PRINTABLE_CAT_H_
#define MCUCORE_SRC_PRINTABLE_CAT_H_

// PrintableCat provides the ability to create a Printable instance that
// concatenates the output from printing *two* or more Printable objects. The
// intent is to use this for creating error messages assembled from multiple
// parts. This is inspired by absl::StrCat, but does not perform allocation.
// Instead it addresses the fact that allocation is "hard" on an Arduino, except
// for stack variables. Therefore it creates an AnyPrintableArray<N>, holding an
// array of N AnyPrintable instances.
//
// Author: james.synge@gmail.com

#include "any_printable.h"
#include "array.h"
#include "mcucore_platform.h"

namespace mcucore {
namespace internal {
size_t PrintAnyPrintablesTo(const AnyPrintable* printables,
                            size_t num_printables, Print& out);
}  // namespace internal

template <size_t SIZE>
struct AnyPrintableArray : public Printable {
  explicit AnyPrintableArray(const Array<AnyPrintable, SIZE>& printables)
      : printables_(printables) {}
  explicit AnyPrintableArray(Array<AnyPrintable, SIZE>&& printables)
      : printables_(printables) {}

  AnyPrintableArray(const AnyPrintableArray<SIZE>&) = default;
  AnyPrintableArray(AnyPrintableArray<SIZE>&&) = default;

  size_t printTo(Print& out) const override {
    return internal::PrintAnyPrintablesTo(printables_.data(), SIZE, out);
  }

  Array<AnyPrintable, SIZE> printables_;  // NOLINT
};

// PrintableCat of two or more values: (a, b, ...)
// Note that we don't support PrintableCat for a single value because that is
// just a Printable. If your single value isn't already a Printable subclass,
// try AnyPrintable.
template <typename... Ts, typename U>
constexpr auto PrintableCat(U u, Ts... ts) {
  return AnyPrintableArray<1 + sizeof...(ts)>(
      MakeArray(AnyPrintable(u), AnyPrintable(ts)...));
}

}  // namespace mcucore

#endif  // MCUCORE_SRC_PRINTABLE_CAT_H_
