#include "printable_cat.h"

#include "absl/strings/str_cat.h"
#include "any_printable.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "gtest/gtest.h"
#include "literal.h"
#include "string_view.h"

namespace mcucore {
namespace test {
namespace {

TEST(PrintableCatTest, Strings) {
  char space = ' ';
  StringView abc("abc");
  Literal def("def");
  AnyPrintable ghi(StringView("ghi"));
  auto p = PrintableCat(abc, space, def, space, ghi);
  EXPECT_EQ(PrintValueToStdString(p), "abc def ghi");
}

TEST(PrintableCatTest, Numbers) {
  char space = ' ';
  auto p = PrintableCat(0, space, 3.1415f, space, 3.1415);
  EXPECT_EQ(PrintValueToStdString(p), absl::StrCat("0 3.14 3.14"));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
