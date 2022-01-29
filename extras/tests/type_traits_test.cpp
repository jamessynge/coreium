#include "type_traits.h"

// Contains tests of the templates in type_traits.h.
//
// This uses multiple distinct namespaces for each the various groups of tests,
// thus allowing for each to define its own helpers as needed without
// interfering with the others.

#include <stdint.h>

#include <map>
#include <vector>

#include "gtest/gtest.h"

namespace mcucore {
namespace test {

struct SomeStructType {};
enum SomeEnumType : int {};
typedef void FunctionType(int);

////////////////////////////////////////////////////////////////////////////////
//
// integral_constant<class T, T v>

namespace {

TEST(TypeTraitsTest, IntegralConstant) {
  // Derived from https://en.cppreference.com/w/cpp/types/integral_constant
  typedef integral_constant<int, 2> two_t;
  typedef integral_constant<int, 4> four_t;
  static_assert(two_t::value * 2 == four_t::value, "2*2 != 4");

  enum class my_e { e1, e2 };
  typedef integral_constant<my_e, my_e::e1> my_e_e1;
  typedef integral_constant<my_e, my_e::e2> my_e_e2;
  static_assert(my_e_e1() == my_e::e1);
  static_assert(my_e_e2() == my_e::e2);
  static_assert(is_same<my_e_e2, my_e_e2>::value, "my_e_e2 != my_e_e2");

#if __cplusplus > 201103L
  // C++ 17 feature
  static_assert(is_same_v<my_e_e1, my_e_e1>, "my_e_e1 != my_e_e1");
#endif  // At least C++ 2017
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
//
// is_same<T, U>

namespace test_is_same {

template <class T>
void VerifyTypeIsSameAsSelf() {
  EXPECT_TRUE((is_same<T, T>::value));

  enum LocalType {};
  EXPECT_TRUE((is_same<LocalType, LocalType>::value));

  EXPECT_FALSE((is_same<T, LocalType>::value));
  EXPECT_FALSE((is_same<LocalType, T>::value));

#if __cplusplus > 201103L
  // is_same_v relies on a C++ 17 feature.
  EXPECT_TRUE((is_same_v<T, T>));
  EXPECT_TRUE((is_same_v<LocalType, LocalType>));
  EXPECT_FALSE((is_same_v<T, LocalType>));
  EXPECT_FALSE((is_same_v<LocalType, T>));
#endif  // At least C++ 2017
}

template <class T, class U>
void VerifyTypesAreNotTheSame() {
  EXPECT_TRUE((is_same<T, T>::value));
  EXPECT_TRUE((is_same<U, U>::value));
  EXPECT_FALSE((is_same<T, U>::value));
  EXPECT_FALSE((is_same<U, T>::value));

#if __cplusplus > 201103L
  // is_same_v relies on a C++ 17 feature.
  EXPECT_FALSE((is_same<T, U>::value));
  EXPECT_FALSE((is_same<U, T>::value));
#endif  // At least C++ 2017
}

TEST(TypeTraitsTest, IsSame) {
  VerifyTypeIsSameAsSelf<bool>();
  VerifyTypeIsSameAsSelf<char>();
  VerifyTypeIsSameAsSelf<int8_t>();
  VerifyTypeIsSameAsSelf<uint8_t>();
  VerifyTypeIsSameAsSelf<int16_t>();
  VerifyTypeIsSameAsSelf<uint16_t>();
  VerifyTypeIsSameAsSelf<int32_t>();
  VerifyTypeIsSameAsSelf<uint32_t>();
  VerifyTypeIsSameAsSelf<short>();           // NOLINT
  VerifyTypeIsSameAsSelf<int>();             // NOLINT
  VerifyTypeIsSameAsSelf<long>();            // NOLINT
  VerifyTypeIsSameAsSelf<unsigned short>();  // NOLINT
  VerifyTypeIsSameAsSelf<unsigned int>();    // NOLINT
  VerifyTypeIsSameAsSelf<unsigned long>();   // NOLINT
  VerifyTypeIsSameAsSelf<float>();
  VerifyTypeIsSameAsSelf<double>();

  enum LocalType {};
  VerifyTypeIsSameAsSelf<LocalType>();

  class Foo {};
  VerifyTypeIsSameAsSelf<Foo>();

  VerifyTypesAreNotTheSame<bool, char>();
  VerifyTypesAreNotTheSame<bool, int8_t>();
  VerifyTypesAreNotTheSame<int8_t, uint8_t>();
  VerifyTypesAreNotTheSame<int8_t, int16_t>();
  VerifyTypesAreNotTheSame<uint8_t, uint16_t>();
  VerifyTypesAreNotTheSame<int16_t, int32_t>();
  VerifyTypesAreNotTheSame<uint16_t, uint32_t>();
  VerifyTypesAreNotTheSame<short, long>();  // NOLINT
  VerifyTypesAreNotTheSame<float, long double>();

  // `int` is implicitly `signed`, but that can be explicitly specified.
  VerifyTypesAreNotTheSame<int, unsigned>();
  VerifyTypesAreNotTheSame<int, unsigned int>();
  VerifyTypesAreNotTheSame<signed int, unsigned>();
  VerifyTypesAreNotTheSame<signed int, unsigned int>();

  // Unlike other types, `char` is neither `unsigned` nor `signed`.
  VerifyTypesAreNotTheSame<char, unsigned char>();
  VerifyTypesAreNotTheSame<char, signed char>();

  // const-qualified type T is not same as non-const T
  VerifyTypesAreNotTheSame<const int, int>();
  VerifyTypesAreNotTheSame<Foo, const Foo>();

  // volatile-qualified type T is not same as non-volatile T
  VerifyTypesAreNotTheSame<volatile int, int>();
  VerifyTypesAreNotTheSame<Foo, volatile Foo>();

  // decltype can supply the type that we compare.
  int int_var1 = 1;
  int int_var2 = 2;
  static_assert(is_same<decltype(int_var1), decltype(int_var2)>::value);

  VerifyTypesAreNotTheSame<int, int*>();
  VerifyTypesAreNotTheSame<int&, int*>();
  int* intptr = &int_var2;
  VerifyTypesAreNotTheSame<decltype(int_var1), decltype(intptr)>();
  static_assert(is_same<decltype(&int_var1), decltype(intptr)>::value);

  // The type of `*var`, given `T var;`, is `T&` rather than `T`.
  VerifyTypesAreNotTheSame<decltype(int_var1), decltype(*intptr)>();
}

}  // namespace test_is_same

////////////////////////////////////////////////////////////////////////////////
//
// Testing void_t, based on examples at
// https://en.cppreference.com/w/cpp/types/void_t

namespace test_void_t {

// Template that checks if a type has begin() and end() member functions.
template <typename, typename = void>
struct is_iterable : false_type {};

template <typename T>
struct is_iterable<
    T, void_t<decltype(declval<T>().begin()), decltype(declval<T>().end())>>
    : true_type {};

struct A {};
struct B {
  void begin();
};
struct C {
  void end();
};
struct D {
  void begin();
  void end();
};

TEST(TypeTraitsTest, VoidT) {
  static_assert(is_iterable<std::vector<double>>::value);
  static_assert(is_iterable<std::map<int, double>>::value);
  static_assert(!is_iterable<double>::value);
  static_assert(!is_iterable<A>::value);
  static_assert(!is_iterable<B>::value);
  static_assert(!is_iterable<C>::value);
  static_assert(is_iterable<D>::value);
}

}  // namespace test_void_t

////////////////////////////////////////////////////////////////////////////////
//
// remove_cv<class T>

namespace test_remove_cv {

static_assert(is_same<remove_cv_t<int>, int>::value);
static_assert(is_same<remove_cv_t<const int>, int>::value);
static_assert(is_same<remove_cv_t<volatile int>, int>::value);
static_assert(is_same<remove_cv_t<const volatile int>, int>::value);
static_assert(
    is_same<remove_cv_t<const volatile int*>, const volatile int*>::value);
static_assert(is_same<remove_cv_t<const int* volatile>, const int*>::value);
static_assert(is_same<remove_cv_t<int* const volatile>, int*>::value);

}  // namespace test_remove_cv

////////////////////////////////////////////////////////////////////////////////
//
// remove_const<class T>

namespace test_remove_const {

static_assert(is_same<remove_const_t<int>, int>::value);
static_assert(is_same<remove_const_t<const int>, int>::value);
static_assert(is_same<remove_const_t<volatile int>, volatile int>::value);
static_assert(is_same<remove_const_t<const volatile int>, volatile int>::value);
static_assert(
    is_same<remove_const_t<const volatile int*>, const volatile int*>::value);
static_assert(
    is_same<remove_const_t<const int* volatile>, const int* volatile>::value);
static_assert(
    is_same<remove_const_t<int* const volatile>, int* volatile>::value);

}  // namespace test_remove_const

////////////////////////////////////////////////////////////////////////////////
//
// remove_volatile<class T>

namespace test_remove_volatile {

static_assert(is_same<remove_volatile_t<int>, int>::value);
static_assert(is_same<remove_volatile_t<const int>, const int>::value);
static_assert(is_same<remove_volatile_t<volatile int>, int>::value);
static_assert(is_same<remove_volatile_t<const volatile int>, const int>::value);
static_assert(is_same<remove_volatile_t<const volatile int*>,
                      const volatile int*>::value);
static_assert(
    is_same<remove_volatile_t<const int* volatile>, const int*>::value);
static_assert(
    is_same<remove_volatile_t<int* const volatile>, int* const>::value);

}  // namespace test_remove_volatile

////////////////////////////////////////////////////////////////////////////////
//
// is_integral<typename T>

namespace test_is_integral {

#define STATIC_ASSERT_IS_INTEGRAL(T)             \
  static_assert(is_integral<T>::value);          \
  static_assert(is_integral<unsigned T>::value); \
  static_assert(is_integral<signed T>::value)

STATIC_ASSERT_IS_INTEGRAL(char);
STATIC_ASSERT_IS_INTEGRAL(short);  // NOLINT
STATIC_ASSERT_IS_INTEGRAL(int);
STATIC_ASSERT_IS_INTEGRAL(long);  // NOLINT

#undef STATIC_ASSERT_IS_INTEGRAL

static_assert(is_integral<bool>::value);
static_assert(is_integral<int8_t>::value);
static_assert(is_integral<uint8_t>::value);
static_assert(is_integral<int16_t>::value);
static_assert(is_integral<uint16_t>::value);
static_assert(is_integral<int32_t>::value);
static_assert(is_integral<uint32_t>::value);

static_assert(!is_integral<float>::value);
static_assert(!is_integral<double>::value);
static_assert(!is_integral<SomeEnumType>::value);
static_assert(!is_integral<SomeStructType>::value);

#if __cplusplus > 201103L
// is_integral_v relies on a C++ 17 feature.

#define STATIC_ASSERT_IS_INTEGRAL_V(T)      \
  static_assert(is_integral_v<T>);          \
  static_assert(is_integral_v<unsigned T>); \
  static_assert(is_integral_v<signed T>)

STATIC_ASSERT_IS_INTEGRAL_V(char);
STATIC_ASSERT_IS_INTEGRAL_V(short);  // NOLINT
STATIC_ASSERT_IS_INTEGRAL_V(int);
STATIC_ASSERT_IS_INTEGRAL_V(long);  // NOLINT

#undef STATIC_ASSERT_IS_INTEGRAL

static_assert(is_integral_v<bool>);
static_assert(is_integral_v<int8_t>);
static_assert(is_integral_v<uint8_t>);
static_assert(is_integral_v<int16_t>);
static_assert(is_integral_v<uint16_t>);
static_assert(is_integral_v<int32_t>);
static_assert(is_integral_v<uint32_t>);

static_assert(!is_integral_v<float>);
static_assert(!is_integral_v<double>);
static_assert(!is_integral_v<SomeEnumType>);
static_assert(!is_integral_v<SomeStructType>);

#endif  // At least C++ 2017

}  // namespace test_is_integral

////////////////////////////////////////////////////////////////////////////////
//
// is_floating_point<typename T>

namespace test_is_floating_point {

static_assert(is_floating_point<float>::value);
static_assert(is_floating_point<double>::value);

static_assert(!is_floating_point<bool>::value);
static_assert(!is_floating_point<char>::value);
static_assert(!is_floating_point<int8_t>::value);
static_assert(!is_floating_point<uint8_t>::value);
static_assert(!is_floating_point<int16_t>::value);
static_assert(!is_floating_point<uint16_t>::value);
static_assert(!is_floating_point<int32_t>::value);
static_assert(!is_floating_point<uint32_t>::value);
static_assert(!is_floating_point<short>::value);           // NOLINT
static_assert(!is_floating_point<int>::value);             // NOLINT
static_assert(!is_floating_point<long>::value);            // NOLINT
static_assert(!is_floating_point<unsigned short>::value);  // NOLINT
static_assert(!is_floating_point<unsigned int>::value);    // NOLINT
static_assert(!is_floating_point<unsigned long>::value);   // NOLINT
static_assert(!is_floating_point<SomeEnumType>::value);
static_assert(!is_floating_point<SomeStructType>::value);

#if __cplusplus > 201103L
// is_integral_v relies on a C++ 17 feature.

static_assert(is_floating_point_v<float>);
static_assert(is_floating_point_v<double>);

static_assert(!is_floating_point_v<bool>);
static_assert(!is_floating_point_v<char>);
static_assert(!is_floating_point_v<int8_t>);
static_assert(!is_floating_point_v<uint8_t>);
static_assert(!is_floating_point_v<int16_t>);
static_assert(!is_floating_point_v<uint16_t>);
static_assert(!is_floating_point_v<int32_t>);
static_assert(!is_floating_point_v<uint32_t>);
static_assert(!is_floating_point_v<short>);           // NOLINT
static_assert(!is_floating_point_v<int>);             // NOLINT
static_assert(!is_floating_point_v<long>);            // NOLINT
static_assert(!is_floating_point_v<unsigned short>);  // NOLINT
static_assert(!is_floating_point_v<unsigned int>);    // NOLINT
static_assert(!is_floating_point_v<unsigned long>);   // NOLINT
static_assert(!is_floating_point_v<SomeEnumType>);
static_assert(!is_floating_point_v<SomeStructType>);

#endif  // At least C++ 2017

}  // namespace test_is_floating_point

////////////////////////////////////////////////////////////////////////////////
//
// is_arithmetic<typename T>

namespace test_is_arithmetic {

#define STATIC_ASSERT_IS_ARITHMETIC(T)             \
  static_assert(is_arithmetic<T>::value);          \
  static_assert(is_arithmetic<unsigned T>::value); \
  static_assert(is_arithmetic<signed T>::value)

STATIC_ASSERT_IS_ARITHMETIC(char);
STATIC_ASSERT_IS_ARITHMETIC(short);  // NOLINT
STATIC_ASSERT_IS_ARITHMETIC(int);
STATIC_ASSERT_IS_ARITHMETIC(long);  // NOLINT

#undef STATIC_ASSERT_IS_ARITHMETIC

static_assert(is_arithmetic<bool>::value);
static_assert(is_arithmetic<int8_t>::value);
static_assert(is_arithmetic<uint8_t>::value);
static_assert(is_arithmetic<int16_t>::value);
static_assert(is_arithmetic<uint16_t>::value);
static_assert(is_arithmetic<int32_t>::value);
static_assert(is_arithmetic<uint32_t>::value);
static_assert(is_arithmetic<float>::value);
static_assert(is_arithmetic<double>::value);

static_assert(!is_arithmetic<SomeEnumType>::value);
static_assert(!is_arithmetic<SomeStructType>::value);

#if __cplusplus > 201103L
// is_integral_v relies on a C++ 17 feature.

#define STATIC_ASSERT_IS_ARITHMETIC_V(T)      \
  static_assert(is_arithmetic_v<T>);          \
  static_assert(is_arithmetic_v<unsigned T>); \
  static_assert(is_arithmetic_v<signed T>)

STATIC_ASSERT_IS_ARITHMETIC_V(char);
STATIC_ASSERT_IS_ARITHMETIC_V(short);  // NOLINT
STATIC_ASSERT_IS_ARITHMETIC_V(int);
STATIC_ASSERT_IS_ARITHMETIC_V(long);  // NOLINT

#undef STATIC_ASSERT_IS_ARITHMETIC

static_assert(is_arithmetic_v<bool>);
static_assert(is_arithmetic_v<int8_t>);
static_assert(is_arithmetic_v<uint8_t>);
static_assert(is_arithmetic_v<int16_t>);
static_assert(is_arithmetic_v<uint16_t>);
static_assert(is_arithmetic_v<int32_t>);
static_assert(is_arithmetic_v<uint32_t>);
static_assert(is_arithmetic_v<float>);
static_assert(is_arithmetic_v<double>);

static_assert(!is_arithmetic_v<SomeEnumType>);
static_assert(!is_arithmetic_v<SomeStructType>);

#endif  // At least C++ 2017

}  // namespace test_is_arithmetic

////////////////////////////////////////////////////////////////////////////////
//
// add_lvalue_reference<typename T>

namespace test_add_lvalue_reference {

// Cases where the result type is an .value reference.
static_assert(is_same<add_lvalue_reference<SomeStructType>::type,
                      SomeStructType&>::value);
static_assert(is_same<add_lvalue_reference<SomeStructType&>::type,
                      SomeStructType&>::value);
static_assert(is_same<add_lvalue_reference<const SomeStructType>::type,
                      const SomeStructType&>::value);
static_assert(is_same<add_lvalue_reference<SomeStructType&&>::type,
                      SomeStructType&>::value);

static_assert(
    is_same<add_lvalue_reference<FunctionType>::type, FunctionType&>::value);

static_assert(is_same<add_lvalue_reference<int>::type, int&>::value);
static_assert(
    is_same<add_lvalue_reference<const int>::type, const int&>::value);

static_assert(is_same<add_lvalue_reference<int&&>::type, int&>::value);

// Can't have a ref to a void, so doesn't add anything.
static_assert(is_same<add_lvalue_reference<void>::type, void>::value);

}  // namespace test_add_lvalue_reference

////////////////////////////////////////////////////////////////////////////////
//
// add_rvalue_reference<typename T>

namespace test_add_rvalue_reference {

// Cases where the result type is an rvalue reference.
static_assert(is_same<add_rvalue_reference<SomeStructType>::type,
                      SomeStructType&&>::value);
static_assert(is_same<add_rvalue_reference<const SomeStructType>::type,
                      const SomeStructType&&>::value);
static_assert(is_same<add_rvalue_reference<SomeStructType&&>::type,
                      SomeStructType&&>::value);
static_assert(is_same<add_rvalue_reference<const SomeStructType&&>::type,
                      const SomeStructType&&>::value);

static_assert(is_same<add_rvalue_reference<bool>::type, bool&&>::value);
static_assert(
    is_same<add_rvalue_reference<const bool>::type, const bool&&>::value);
static_assert(is_same<add_rvalue_reference<bool&&>::type, bool&&>::value);
static_assert(
    is_same<add_rvalue_reference<const bool&&>::type, const bool&&>::value);

// Cases where the result type is NOT an rvalue reference.

static_assert(is_same<add_rvalue_reference<SomeStructType&>::type,
                      SomeStructType&>::value);
static_assert(is_same<add_rvalue_reference<int&>::type, int&>::value);

// Can't have a ref to a void, so doesn't add anything.
static_assert(is_same<add_rvalue_reference<void>::type, void>::value);

}  // namespace test_add_rvalue_reference

////////////////////////////////////////////////////////////////////////////////
//
// declval<typename T>
//
// Tests based on https://en.cppreference.com/w/cpp/utility/declval

namespace test_declval {

struct NonDefault {
  NonDefault() = delete;
  int foo() const { return 1; }
};

// Can't compile this because NonDefault doesn't have a default ctor:
//   static_assert(is_same<decltype(NonDefault().foo()), int>::value);
// So we use declval to "make" a type that can be created via a default ctor.
static_assert(is_same<decltype(declval<NonDefault>().foo()), int>::value);

}  // namespace test_declval

////////////////////////////////////////////////////////////////////////////////
//
// is_pointer<class T>
//
// Tests based on https://en.cppreference.com/w/cpp/types/is_pointer

namespace test_is_pointer {

class A {};

static_assert(is_pointer<A*>::value);
static_assert(is_pointer<A const* volatile>());
static_assert(is_pointer<int*>::value);
static_assert(is_pointer<int**>::value);

static_assert(!is_pointer<A>::value);
static_assert(!is_pointer<A>());    // Using inherited operator bool
static_assert(!is_pointer<A>{});    // Ditto
static_assert(!is_pointer<A>()());  // Using inherited operator()
static_assert(!is_pointer<A>{}());  // Ditto
static_assert(!is_pointer<A&>::value);
static_assert(!is_pointer<int>::value);
static_assert(!is_pointer<int[10]>::value);
static_assert(!is_pointer<std::nullptr_t>::value);

#if __cplusplus > 201103L
// is_pointer_v relies on a C++ 17 feature.
static_assert(!is_pointer_v<A>);
#endif

}  // namespace test_is_pointer

////////////////////////////////////////////////////////////////////////////////
//
// enable_if<bool B, class T = void>
//
// Usually used to enable a class specialization, but this example is quite
// interesting too: https://stackoverflow.com/a/21464113

namespace test_enable_if {

class foo;
class bar;

template <class T>
struct is_bar {
  // This specialization of check exists if T is bar.
  template <class Q = T>
  typename enable_if<is_same<Q, bar>::value, bool>::type check() {
    return true;
  }

  // This specialization of check exists if T is not bar.
  template <class Q = T>
  typename enable_if<!is_same<Q, bar>::value, bool>::type check() {
    return false;
  }
};

TEST(TypeTraitsTest, EnableIf) {
  is_bar<bar> bar_is_bar;
  EXPECT_TRUE(bar_is_bar.check());
  is_bar<foo> foo_is_bar;
  EXPECT_FALSE(foo_is_bar.check());
}

}  // namespace test_enable_if

////////////////////////////////////////////////////////////////////////////////
//
// is_signed<typename T>

namespace test_is_signed {

#define STATIC_ASSERT_IS_SIGNED(T)              \
  static_assert(is_signed<T>::value);           \
  static_assert(!is_signed<unsigned T>::value); \
  static_assert(is_signed<signed T>::value)

STATIC_ASSERT_IS_SIGNED(short);  // NOLINT
STATIC_ASSERT_IS_SIGNED(int);
STATIC_ASSERT_IS_SIGNED(long);   // NOLINT
STATIC_ASSERT_IS_SIGNED(short);  // NOLINT
STATIC_ASSERT_IS_SIGNED(int);
STATIC_ASSERT_IS_SIGNED(long);  // NOLINT

#undef STATIC_ASSERT_IS_SIGNED

static_assert(is_signed<int8_t>::value);
static_assert(is_signed<int16_t>::value);
static_assert(is_signed<int32_t>::value);
static_assert(is_signed<float>::value);
static_assert(is_signed<double>::value);

static_assert(!is_signed<bool>::value);
static_assert(!is_signed<uint8_t>::value);
static_assert(!is_signed<uint16_t>::value);
static_assert(!is_signed<uint32_t>::value);
static_assert(!is_signed<SomeEnumType>::value);
static_assert(!is_signed<SomeStructType>::value);

#if __cplusplus > 201103L
// is_signed_v relies on a C++ 17 feature.

#define STATIC_ASSERT_IS_SIGNED_V(T)       \
  static_assert(is_signed_v<T>);           \
  static_assert(!is_signed_v<unsigned T>); \
  static_assert(is_signed_v<signed T>)

STATIC_ASSERT_IS_SIGNED_V(short);  // NOLINT
STATIC_ASSERT_IS_SIGNED_V(int);
STATIC_ASSERT_IS_SIGNED_V(long);   // NOLINT
STATIC_ASSERT_IS_SIGNED_V(short);  // NOLINT
STATIC_ASSERT_IS_SIGNED_V(int);
STATIC_ASSERT_IS_SIGNED_V(long);  // NOLINT

#undef STATIC_ASSERT_IS_SIGNED_V

static_assert(is_signed_v<int8_t>);
static_assert(is_signed_v<int16_t>);
static_assert(is_signed_v<int32_t>);
static_assert(is_signed_v<float>);
static_assert(is_signed_v<double>);

static_assert(!is_signed_v<bool>);
static_assert(!is_signed_v<uint8_t>);
static_assert(!is_signed_v<uint16_t>);
static_assert(!is_signed_v<uint32_t>);
static_assert(!is_signed_v<SomeEnumType>);
static_assert(!is_signed_v<SomeStructType>);

#endif  // At least C++ 2017

}  // namespace test_is_signed

}  // namespace test
}  // namespace mcucore
