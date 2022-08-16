#include "print/has_insert_into.h"

#include "extras/test_tools/print_to_std_string.h"
#include "gtest/gtest.h"
#include "print/o_print_stream.h"

namespace mcucore {
namespace test {
namespace {

struct Unsupported {};
static_assert(has_insert_into<Unsupported>::value == false);

struct WrongSignature1 {
  void InsertInto() {}
};
static_assert(has_insert_into<WrongSignature1>::value == false);

struct WrongSignature2 {
  void InsertInto(OPrintStream& a, bool b) {}
};
static_assert(has_insert_into<WrongSignature2>::value == false);

struct WrongSignature3 {
  void InsertInto(OPrintStream a) {}
};
static_assert(has_insert_into<WrongSignature3>::value == false);

struct Supported {
  void InsertInto(OPrintStream& a) const { a << "yes!"; }
};
static_assert(has_insert_into<Supported>::value == true);

TEST(HasInsertIntoTest, ConfirmStaticAsserts) {
  PrintToStdString p2ss;
  OPrintStream out(p2ss);
  out << Supported();
  EXPECT_EQ(p2ss.str(), "yes!");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
