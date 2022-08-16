#include "print/printable_cat.h"

#include "extras/test_tools/print_value_to_std_string.h"
#include "gtest/gtest.h"
#include "print/any_printable.h"
#include "strings/progmem_string.h"
#include "strings/progmem_string_data.h"
#include "strings/progmem_string_view.h"
#include "strings/string_view.h"

namespace mcucore {
namespace test {
namespace {

enum class SomeEnum : uint32_t {
  kMin = 0,
  kMax = std::numeric_limits<uint32_t>::max(),
};

TEST(PrintableCatTest, Strings) {
  char space = ' ';
  StringView abc("abc");
  ProgmemStringView def("def");
  AnyPrintable ghi(StringView(" ghi"));
  ProgmemString jkl = MCU_PSD(" jkl");
  auto p = PrintableCat(abc, space, def, ghi, jkl, MCU_FLASHSTR(" mno"));
  EXPECT_EQ(PrintValueToStdString(p), "abc def ghi jkl mno");
}

TEST(PrintableCatTest, Numbers) {
  char space = ' ';
  auto p = PrintableCat(0, space, 3.1415f, space, 3.1415);
  EXPECT_EQ(PrintValueToStdString(p), "0 3.14 3.14");
}

TEST(PrintableCatTest, Mixture) {
  char space = ' ';
  auto p = PrintableCat(MCU_FLASHSTR("Msg:"), space, SomeEnum::kMin, space,
                        SomeEnum::kMax, space, static_cast<SomeEnum>(123),
                        space, StringView("sv"));
  EXPECT_EQ(PrintValueToStdString(p), "Msg: 0 4294967295 123 sv");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
