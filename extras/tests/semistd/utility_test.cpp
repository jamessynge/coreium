#include "semistd/utility.h"

#include <iostream>

#include "extras/test_tools/pretty_type_name.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

template <typename T>
bool got_rvalue_ref(T&&) {
  return true;
}

template <typename T>
bool got_rvalue_ref(T&) {
  return false;
}

template <typename T>
bool was_forwarded_as_rvalue_ref(T&& param) {
  std::cout << "T is " << PrettyTypeName<T>() << std::endl << std::flush;
  return got_rvalue_ref(forward<T>(param));
}

TEST(UtilityTest, ForwardInt) {
  int var = 0;
  EXPECT_FALSE(was_forwarded_as_rvalue_ref(var));
  EXPECT_TRUE(was_forwarded_as_rvalue_ref(0));
}

struct A {};
struct B {};
struct C {};
struct D {};

int Write(A& a, B b, const C& c, D&& d, int result) { return result; }

template <typename F, typename... T>
int CallWrite(F f, T&&... t) {
  return f(forward<T>(t)...);
}

TEST(UtilityTest, ForwardFunc) {
  A a;
  const C c;
  D d;
  EXPECT_EQ(CallWrite(Write, a, B(), c, move(d), 123), 123);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
