#include "printable_cat.h"

#include "absl/strings/str_cat.h"
#include "any_printable.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "gtest/gtest.h"
#include "progmem_string.h"
#include "progmem_string_data.h"
#include "progmem_string_view.h"
#include "string_view.h"

namespace mcucore {
namespace test {
namespace {

TEST(PrintableCatTest, Strings) {
  char space = ' ';
  StringView abc("abc");
  ProgmemStringView def("def");
  AnyPrintable ghi(StringView(" ghi"));
  ProgmemString jkl = MCU_PSD(" jkl");
  auto p = PrintableCat(abc, space, def, ghi, jkl);
  EXPECT_EQ(PrintValueToStdString(p), "abc def ghi jkl");
}

TEST(PrintableCatTest, Numbers) {
  char space = ' ';
  auto p = PrintableCat(0, space, 3.1415f, space, 3.1415);
  EXPECT_EQ(PrintValueToStdString(p), absl::StrCat("0 3.14 3.14"));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
