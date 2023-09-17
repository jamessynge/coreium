#ifndef MCUCORE_SRC_SEMISTD_TUPLE_H_
#define MCUCORE_SRC_SEMISTD_TUPLE_H_

// A partial implementation of std::tuple, sufficient to support PrintableCat,
// and maybe more.
//
// Initially this is based on https://stackoverflow.com/a/52208842. See also
// https://github.com/Quuxplusone/from-scratch/blob/master/include/scratch/bits/tuple/tuple.h
// and
// http://mitchnull.blogspot.com/2012/06/c11-tuple-implementation-details-part-1.html.

#include "mcucore_platform.h"
#include "semistd/tuple.h"

namespace mcucore {
namespace tuple_internal {

// Contains the actual value for one item in the tuple. The template parameter
// `i` allows the `tuple_get` function to find the value in O(1) time.
template <size_t i, typename Item>
struct tuple_leaf {
  tuple_leaf() = default;
  explicit tuple_leaf(Item item) : value(move(item)) {}
  explicit tuple_leaf(const Item& item) : value(item) {}

  Item value;
};

// tuple_impl is a proxy for the final class that has an extra
// template parameter `i`.
template <size_t i, typename... Items>
struct tuple_impl;

// Base case: empty tuple
template <size_t i>
struct tuple_impl<i> {};

// Recursive specialization
template <size_t i, typename HeadItem, typename... TailItems>
struct tuple_impl<i, HeadItem, TailItems...>
    : public tuple_leaf<i, HeadItem>,  // This adds a `value` member of type
                                       // HeadItem
      public tuple_impl<i + 1, TailItems...>  // This recurses
{
  using A = tuple_leaf<i, HeadItem>;
  using B = tuple_impl<i + 1, TailItems...>;
  tuple_impl(HeadItem h, TailItems... ts) : A(h), B(ts...) {}  // NOLINT
};

// Obtain a reference to i-th item in a tuple
template <size_t i, typename HeadItem, typename... TailItems>
HeadItem& tuple_get(tuple_impl<i, HeadItem, TailItems...>& tuple) {
  // Fully qualified name for the member, to find the right one
  // (they are all called `value`).
  return tuple.tuple_leaf<i, HeadItem>::value;
}

// Templated alias to avoid having to specify `i = 0`
template <typename... Items>
using Tuple = tuple_impl<0, Items...>;

}  // namespace tuple_internal

// Templated alias to avoid having to specify `i = 0`
template <typename... Items>
using tuple = tuple_internal::tuple_impl<0, Items...>;

// Very simplified version, without any use of std::forward, etc.
template <class... Types>
tuple<Types...> make_tuple(Types&&... args) {
  return tuple<Types...>(args...);
}

}  // namespace mcucore

#endif  // MCUCORE_SRC_SEMISTD_TUPLE_H_
