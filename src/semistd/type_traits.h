#ifndef MCUCORE_SRC_SEMISTD_TYPE_TRAITS_H_
#define MCUCORE_SRC_SEMISTD_TYPE_TRAITS_H_

// The parts of STL's <type_traits>, etc., needed for McuCore, McuNet and Tiny
// Alpaca Server. This exists because McuCore et all are targeted at the Arduino
// platform, which in the case of the Arduino Uno and Mega, means compiling with
// avr-gcc, which is mostly C++ 11 compatible, but without the C++ Standard
// Library.
//
// Note that `long long` and `long double` types are not supported here because
// they're unlikely to be supported by the compiler and don't make much sense in
// the typical microcontroller setting.
//
// FYI, another interesting implementation of <type_traits> can be found here:
//
//    https://github.com/Quuxplusone/from-scratch/blob/master/include/scratch/bits/type-traits
//
// Author: james.synge@gmail.com (with plenty of material copied from elsewhere,
// see below for details)

#include "mcucore_platform.h"

namespace mcucore {

////////////////////////////////////////////////////////////////////////////////
//
// integral_constant<class T, T v>
//
// integral_constant wraps a static constant of specified type. It is the base
// class for the C++ type traits. Two typedefs for the common case where T is
// bool are provided.
//
// Based on https://en.cppreference.com/w/cpp/types/integral_constant

template <class T, T t_value>
struct integral_constant {
  static constexpr T value = t_value;
  using value_type = T;
  using type = integral_constant;  // using injected-class-name
  constexpr operator value_type() const noexcept { return value; }  // NOLINT

  // Since C++ 14.
  constexpr value_type operator()() const noexcept { return value; }  // NOLINT
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

////////////////////////////////////////////////////////////////////////////////
//
// is_const<typename T>
//
// If T is a const-qualified type (that is, const, or const volatile), provides
// the member constant value equal to true. For any other type, value is false.
//
// Notes: If T is a reference type then is_const<T>::value is always false. The
// proper way to check a potentially-reference type for const-ness is to remove
// the reference: is_const<typename remove_reference<T>::type>
//
// Based on https://en.cppreference.com/w/cpp/types/is_const

template <class T>
struct is_const : false_type {};

template <class T>
struct is_const<const T> : true_type {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <class T>
inline constexpr bool is_const_v = is_const<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// is_same<T, U>
//
// If T and U name the same type (taking into account const/volatile
// qualifications), provides the member constant value equal to true. Otherwise
// value is false.
//
// Based on https://en.cppreference.com/w/cpp/types/is_same

template <class T, class U>
struct is_same : false_type {};

template <class T>
struct is_same<T, T> : true_type {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <class T, class U>
inline constexpr bool is_same_v = is_same<T, U>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// void_t<class...>
//
// Utility metafunction that maps a sequence of any types to the type void. This
// metafunction is a convenient way to leverage SFINAE prior to C++20's
// concepts, in particular for conditionally remove functions from the candidate
// set based on whether an expression is valid in the unevaluated context (such
// as operand to decltype expression), allowing there to exist separate function
// overloads or specializations, based on supported operations.
//
// Based on https://en.cppreference.com/w/cpp/types/void_t

namespace tt_internal {
template <typename... Ts>
struct make_void {
  typedef void type;
};
}  // namespace tt_internal

template <typename... Ts>
using void_t = typename tt_internal::make_void<Ts...>::type;

////////////////////////////////////////////////////////////////////////////////
//
// remove_cv<class T>
//
//    Provides the member typedef type which is the same as T, except that its
//    topmost const, topmost volatile, or both, are removed, if present.
//
// remove_const<class T>
//
//    Provides the member typedef type which is the same as T, except that its
//    topmost const qualifier is removed, if present.
//
// remove_volatile<class T>
//
//    Provides the member typedef type which is the same as T, except that its
//    topmost volatile qualifier is removed, if present.
//
// Based on https://en.cppreference.com/w/cpp/types/remove_cv

template <class T>
struct remove_cv {
  typedef T type;
};
template <class T>
struct remove_cv<const T> {
  typedef T type;
};
template <class T>
struct remove_cv<volatile T> {
  typedef T type;
};
template <class T>
struct remove_cv<const volatile T> {
  typedef T type;
};

template <class T>
struct remove_const {
  typedef T type;
};
template <class T>
struct remove_const<const T> {
  typedef T type;
};

template <class T>
struct remove_volatile {
  typedef T type;
};
template <class T>
struct remove_volatile<volatile T> {
  typedef T type;
};

template <class T>
using remove_cv_t = typename remove_cv<T>::type;
template <class T>
using remove_const_t = typename remove_const<T>::type;
template <class T>
using remove_volatile_t = typename remove_volatile<T>::type;

////////////////////////////////////////////////////////////////////////////////
//
// is_void<typename T>
//
// Checks whether T is a void type. Provides the member constant value that is
// equal to true, if T is the type void, const void, volatile void, or const
// volatile void. Otherwise, value is equal to false.
//
// Based on https://en.cppreference.com/w/cpp/types/is_void

template <typename T>
struct is_void : is_same<void, typename remove_cv<T>::type> {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <typename T>
inline constexpr bool is_void_v = is_void<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// is_null_pointer<typename T>
//
// Checks whether T is the type nullptr_t. Provides the member constant value
// that is equal to true, if T is the type nullptr_t, const nullptr_t, volatile
// nullptr_t, or const volatile nullptr_t. Otherwise, value is equal to false.
//
// Based on https://en.cppreference.com/w/cpp/types/is_void

template <typename T>
struct is_null_pointer : is_same<nullptr_t, remove_cv_t<T>> {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <typename T>
inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// remove_reference<typename T>
//
// If the type T is a reference type, provides the member typedef type which is
// the type referred to by T. Otherwise type is T.
//
// Based on https://en.cppreference.com/w/cpp/types/remove_reference

template <typename T>
struct remove_reference {
  typedef T type;
};
template <typename T>
struct remove_reference<T&> {
  typedef T type;
};
template <typename T>
struct remove_reference<T&&> {
  typedef T type;
};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

////////////////////////////////////////////////////////////////////////////////
//
// is_integral<typename T>
//
// Checks whether T is an integral type. Provides the member constant value
// which is equal to true, if T is the type bool, int8_t, uint8_t, int16_t,
// uint16_t, int32_t, uint32_t (other integral types can be added below as they
// become available on the supported platforms). Otherwise, value is equal to
// false.
//
// Based on https://stackoverflow.com/a/43571992
// Behaves mostly like std::is_integral ...

namespace tt_internal {

template <class T>
struct is_integral_helper
    : integral_constant<bool, is_same<bool, T>::value                  //
                                  || is_same<char, T>::value           //
                                  || is_same<unsigned char, T>::value  //
                                  || is_same<signed char, T>::value    //
                                  || is_same<short, T>::value          // NOLINT
                                  ||
                                  is_same<unsigned short, T>::value    // NOLINT
                                  || is_same<int, T>::value            // NOLINT
                                  || is_same<unsigned int, T>::value   //
                                  || is_same<long, T>::value           // NOLINT
                                  || is_same<unsigned long, T>::value  // NOLINT
#ifdef INT8_MAX
                                  || is_same<int8_t, T>::value
#endif
#ifdef UINT8_MAX
                                  || is_same<uint8_t, T>::value
#endif
#ifdef INT16_MAX
                                  || is_same<int16_t, T>::value
#endif
#ifdef UINT16_MAX
                                  || is_same<uint16_t, T>::value
#endif
#ifdef INT32_MAX
                                  || is_same<int32_t, T>::value
#endif
#ifdef UINT32_MAX
                                  || is_same<uint32_t, T>::value
#endif
#ifdef INT64_MAX
                                  || is_same<int64_t, T>::value
#endif
#ifdef UINT64_MAX
                                  || is_same<uint64_t, T>::value
#endif
                        > {
};

}  // namespace tt_internal

// is_integral extends either true_type or false_type, depending on whether T
// is an integral type.
template <class T>
struct is_integral
    : tt_internal::is_integral_helper<typename remove_cv<T>::type> {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// is_floating_point<typename T>
//
// Checks whether T is a floating-point type. Provides the member constant value
// which is equal to true, if T is the type float, double, long double,
// including any cv-qualified variants. Otherwise, value is equal to false.
//
// Based on https://stackoverflow.com/a/43571992
// Behaves mostly like std::is_floating_point

namespace tt_internal {

template <typename>
struct test_is_floating_point : false_type {};

#define TYPE_TRAITS_IS_FLOATING_POINT_HELPER_(base_name) \
  template <>                                            \
  struct test_is_floating_point<base_name> : true_type {}

TYPE_TRAITS_IS_FLOATING_POINT_HELPER_(float);
TYPE_TRAITS_IS_FLOATING_POINT_HELPER_(double);
// TYPE_TRAITS_IS_FLOATING_POINT_HELPER_(long double);

#undef TYPE_TRAITS_IS_FLOATING_POINT_HELPER_
}  // namespace tt_internal

template <typename T>
struct is_floating_point
    : decltype(tt_internal::test_is_floating_point<remove_cv_t<T>>()) {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <typename T>
inline constexpr bool is_floating_point_v = is_floating_point<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// is_arithmetic<typename T>
//
// If T is an arithmetic type (that is, an integral type or a floating-point
// type) or a cv-qualified version thereof, provides the member constant value
// equal to true. For any other type, value is false.
//
// Arithmetic types are the built-in types for which the arithmetic operators
// (+, -, *, /) are defined
//
// Based on https://en.cppreference.com/w/cpp/types/is_arithmetic

template <typename T>
struct is_arithmetic
    : integral_constant<bool, is_integral<T>::value ||
                                  is_floating_point<T>::value> {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <typename T>
inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// add_lvalue_reference<class T>
//
//    If T is a function type that has no cv- or ref- qualifier or an object
//    type, provides a member typedef type which is T&. If T is an rvalue
//    reference to some type U, then type is U&. Otherwise, type is T.
//
// add_rvalue_reference<class T>
//
//    If T is a function type that has no cv- or ref- qualifier or an object
//    type, provides a member typedef type which is T&&, otherwise type is T.
//
// Note that the standard allows for rather odd types, such as const int&&,
// which have some uses; for more info: https://stackoverflow.com/a/60587511.
//
// Based on https://en.cppreference.com/w/cpp/types/add_reference:

namespace tt_internal {

template <class T>
struct type_identity {
  using type = T;
};

template <class T>
auto try_add_lvalue_reference(int) -> type_identity<T&>;
template <class T>
auto try_add_lvalue_reference(...) -> type_identity<T>;

template <class T>
auto try_add_rvalue_reference(int) -> type_identity<T&&>;
template <class T>
auto try_add_rvalue_reference(...) -> type_identity<T>;

}  // namespace tt_internal

template <class T>
struct add_lvalue_reference
    : decltype(tt_internal::try_add_lvalue_reference<T>(0)) {};

template <class T>
struct add_rvalue_reference
    : decltype(tt_internal::try_add_rvalue_reference<T>(0)) {};

template <class T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;
template <class T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

////////////////////////////////////////////////////////////////////////////////
//
// declval<class T>
//
// Converts any type T to a reference type, making it possible to use member
// functions in decltype expressions without the need to go through
// constructors. declval is commonly used in templates where acceptable template
// parameters may have no constructor in common, but have the same member
// function whose return type is needed. Note that declval can only be used in
// unevaluated contexts and is not required to be defined; it is an error to
// evaluate an expression that contains this function.
//
// Based on https://en.cppreference.com/w/cpp/utility/declval

template <class T>
typename add_rvalue_reference<T>::type declval() noexcept;

////////////////////////////////////////////////////////////////////////////////
//
// is_pointer<class T>
//
// Checks whether T is a pointer to object or a pointer to function (but not a
// pointer to member/member function) or a cv-qualified version thereof.
// Provides the member constant value which is equal to true, if T is a
// object/function pointer type. Otherwise, value is equal to false.
//
// Based on https://en.cppreference.com/w/cpp/types/is_pointer

namespace tt_internal {

template <class T>
struct is_pointer_helper : false_type {};

template <class T>
struct is_pointer_helper<T*> : true_type {};

}  // namespace tt_internal

template <class T>
struct is_pointer
    : tt_internal::is_pointer_helper<typename remove_cv<T>::type> {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <class T>
inline constexpr bool is_pointer_v = is_pointer<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// sfinae_true<class>
//
// This supports testing whether a type has a member. See has_print_to.h for an
// example of applying this, or read https://stackoverflow.com/a/9154394 and
// http://coliru.stacked-crooked.com/a/cd139d95d214c5c3. Note that this is NOT
// defined in the C++ standard.

template <class>
struct sfinae_true : true_type {};

////////////////////////////////////////////////////////////////////////////////
//
// enable_if<bool B, class T = void>
//
// If B is true, enable_if has a public member typedef type, equal to T;
// otherwise, there is no member typedef.
//
// This metafunction is a convenient way to leverage SFINAE prior to C++20's
// concepts, in particular for conditionally removing functions from the
// candidate set based on type traits, allowing separate function overloads or
// specializations based on those different type traits.
//
// enable_if can be used in many forms, including:
//
// * as an additional function argument (not applicable to operator overloads)
// * as a return type (not applicable to constructors and destructors)
// * as a class template or function template parameter

// Based on https://en.cppreference.com/w/cpp/types/enable_if

template <bool B, class T = void>
struct enable_if {};

template <class T>
struct enable_if<true, T> {
  typedef T type;
};

template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

////////////////////////////////////////////////////////////////////////////////
//
// is_signed<typename T>
//
// If T is an arithmetic type that can have negative values, provides the member
// constant value equal to true: this results in true for the floating-point
// types and the signed integer types, and in false for the unsigned integer
// types and the type bool. For any other type, value is false.
//
// Based on https://en.cppreference.com/w/cpp/types/is_signed

namespace tt_internal {
template <typename T, bool = is_arithmetic<T>::value>
struct is_signed : integral_constant<bool, (T(-1) < T(0))> {};

template <typename T>
struct is_signed<T, false> : false_type {};
}  // namespace tt_internal

template <typename T>
struct is_signed : tt_internal::is_signed<T>::type {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <class T>
inline constexpr bool is_signed_v = is_signed<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// is_array<typename T>
//
// Checks whether T is an array type. Provides the member constant value which
// is equal to true, if T is an array type. Otherwise, value is equal to false.

template <class T>
struct is_array : false_type {};

template <class T>
struct is_array<T[]> : true_type {};

template <class T, size_t N>
struct is_array<T[N]> : true_type {};

////////////////////////////////////////////////////////////////////////////////
//
// is_reference<typename T>
//
// If T is a reference type (lvalue reference or rvalue reference), provides the
// member constant value equal true. For any other type, value is false.

template <class T>
struct is_reference : false_type {};
template <class T>
struct is_reference<T&> : true_type {};
template <class T>
struct is_reference<T&&> : true_type {};

////////////////////////////////////////////////////////////////////////////////
//
// is_function<typename T>
//
// Checks whether T is a function type. Types like std::function, lambdas,
// classes with overloaded operator() and pointers to functions don't count as
// function types. Provides the member constant value which is equal to true, if
// T is a function type. Otherwise, value is equal to false.
//
// Discussion here: https://stackoverflow.com/q/59654482
//
// Based on https://en.cppreference.com/w/cpp/types/is_function

template <class T>
struct is_function : integral_constant<bool, !is_const<const T>::value &&
                                                 !is_reference<T>::value> {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <class T>
inline constexpr bool is_function_v = is_function<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// is_member_pointer<typename T>
//
// If T is pointer to non-static member object or a pointer to non-static member
// function, provides the member constant value equal true. For any other type,
// value is false.
//
// Based on https://en.cppreference.com/w/cpp/types/is_member_pointer

namespace tt_internal {
template <class T>
struct is_member_pointer_helper : false_type {};

template <class T, class U>
struct is_member_pointer_helper<T U::*> : true_type {};
}  // namespace tt_internal

template <class T>
struct is_member_pointer
    : tt_internal::is_member_pointer_helper<typename remove_cv<T>::type> {};

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <class T>
inline constexpr bool is_member_pointer_v = is_member_pointer<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// is_union_or_class<typename T>
//
// This is non-standard, but doesn't require compiler intrinsics, and is useful
// below for implementing is_enum without the need for compiler intrinsics.
// Based on discussions on stackoverflow, including:
// https://stackoverflow.com/a/45000642

namespace tt_internal {

template <class T, typename P = int T::*>
struct is_union_or_class_helper : true_type {};

template <class T, typename P = int T::*>
auto test_is_union_or_class(int) -> is_union_or_class_helper<T, P>;

template <class T>
auto test_is_union_or_class(...) -> false_type;

}  // namespace tt_internal

template <typename T>
struct is_union_or_class : decltype(tt_internal::test_is_union_or_class<T>(0)) {
};

////////////////////////////////////////////////////////////////////////////////
//
// is_enum<typename T>
//
// Checks whether T is an enumeration type. Provides the member constant value
// which is equal to true, if T is an enumeration type. Otherwise, value is
// equal to false.
//
// std::is_enum<T> depends on compiler intrinsics, i.e. non-standardized
// features. If those features are available, this code uses them, but it falls
// back to an approach found here: https://stackoverflow.com/a/11317409

#if MCU_HAS_FEATURE(is_enum)

template <typename T>
struct is_enum : integral_constant<bool, __is_enum(T)> {};

#else

template <typename T>
struct is_enum
    : public integral_constant<bool,
                               !is_void<T>::value                   //
                                   && !is_null_pointer<T>::value    //
                                   && !is_integral<T>::value        //
                                   && !is_floating_point<T>::value  //
                                   && !is_array<T>::value           //
                                   && !is_pointer<T>::value         //
                                   && !is_reference<T>::value       //
                                   && !is_member_pointer<T>::value  //
                                   && !is_union_or_class<T>::value  //
                                   && !is_function<T>::value> {};

#endif

#ifdef MCU_CXX14_VARIABLE_TEMPLATES
// C++ 17 feature, and relies on features of C++ 14.
template <typename T>
inline constexpr bool is_enum_v = is_enum<T>::value;
#endif  // At least C++ 2017

////////////////////////////////////////////////////////////////////////////////
//
// underlying_type<typename T>
//
// If T is a complete enumeration (enum) type, provides a member typedef type
// that names the underlying type of T. Otherwise, if T is not an enumeration
// type, there is no member type. Otherwise (T is an incomplete enumeration
// type), the program is ill-formed.
//
// std::underlying_type<T> depends on compiler intrinsics, i.e. non-standardized
// features. If those features are available, this code uses them; if not, this
// code provides an approximation of the intended behavior: it produces an
// integer type which has correct size for storing the enum, and *probably* the
// correct sign. For example:
//
// * On a system where int is 32-bits, underlying_type will return uint32_t for
//   an enum whose underlying type is explicitly specified as unsigned.
//
// * If the underlying type is not explicitly specified in the enum declaration,
//   and none of the enumerator values is negative, the compiler may choose an
//   underlying type of `unsigned`, yet the is_signed_enum template below may
//   still determine that the type is signed. Sigh.

namespace tt_underlying_type {

template <typename T, bool IsSigned, int Size>
struct get_underlying_type {};

template <typename T>
struct get_underlying_type<T, true, 1> {
  using type = int8_t;
};

template <typename T>
struct get_underlying_type<T, false, 1> {
  using type = uint8_t;
};

template <typename T>
struct get_underlying_type<T, true, 2> {
  using type = int16_t;
};

template <typename T>
struct get_underlying_type<T, false, 2> {
  using type = uint16_t;
};

template <typename T>
struct get_underlying_type<T, true, 4> {
  using type = int32_t;
};

template <typename T>
struct get_underlying_type<T, false, 4> {
  using type = uint32_t;
};

template <typename T>
struct get_underlying_type<T, true, 8> {
  using type = int64_t;
};

template <typename T>
struct get_underlying_type<T, false, 8> {
  using type = uint64_t;
};

// This doesn't work (portably?) for implicitly unsigned enums; it may determine
// that they are signed.
template <typename T>
struct is_signed_enum
    : integral_constant<bool, (static_cast<T>(-1) < static_cast<T>(0))> {};

template <typename T, bool Condition>
struct maybe_underlying_type {};

#if MCU_HAS_FEATURE(underlying_type)

template <typename T>
struct maybe_underlying_type<T, true> {
  using type = __underlying_type(T);
};

#else  // !MCU_HAS_FEATURE(underlying_type)
template <typename T>
struct maybe_underlying_type<T, true>
    : get_underlying_type<T, is_signed_enum<T>::value, sizeof(T)> {};

#endif  // MCU_HAS_FEATURE(underlying_type)

}  // namespace tt_underlying_type

template <typename T>
struct underlying_type
    : tt_underlying_type::maybe_underlying_type<T, is_enum<T>::value> {};

template <class T>
using underlying_type_t = typename underlying_type<T>::type;

}  // namespace mcucore

#endif  // MCUCORE_SRC_SEMISTD_TYPE_TRAITS_H_
