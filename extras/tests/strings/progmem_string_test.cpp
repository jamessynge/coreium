#include "strings/progmem_string.h"

#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/test_strings.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "strings/progmem_string_data.h"

namespace mcucore {
namespace test {
namespace {

TEST(ProgmemStringTest, FromFlashString) {
  {
    ProgmemString ps(FLASHSTR("abc"));
    EXPECT_EQ(PrintValueToStdString(ps), "abc");
  }
  {
    ProgmemString ps(MCU_FLASHSTR("ABC"));
    EXPECT_EQ(PrintValueToStdString(ps), "ABC");
  }
}

TEST(ProgmemStringTest, FromProgmemStringData) {
  {
    progmem_string_data::ProgmemStringData<'f', 'o', 'o'> psd;
    ProgmemString ps(psd);
    EXPECT_EQ(PrintValueToStdString(ps), "foo");
  }
  {
    ProgmemString ps(MCU_PSD(TEST_STR_64));
    EXPECT_EQ(PrintValueToStdString(ps), TEST_STR_64);
  }
}

TEST(ProgmemStringTest, CopyCtor) {
  ProgmemString ps1(MCU_PSD("foobar"));
  ProgmemString ps2(ps1);
  EXPECT_EQ(PrintValueToStdString(ps1), "foobar");
  EXPECT_EQ(PrintValueToStdString(ps2), "foobar");
  EXPECT_EQ(ps1.ToFlashStringHelper(), ps2.ToFlashStringHelper());
}

TEST(ProgmemStringTest, Comparison) {
  ProgmemString ps1(MCU_PSD("foobar"));
  ProgmemString ps2(MCU_PSD("foobar"));
  EXPECT_EQ(PrintValueToStdString(ps1), "foobar");
  EXPECT_EQ(PrintValueToStdString(ps2), "foobar");
  EXPECT_EQ(ps1, ps2);
  EXPECT_EQ(ps1.ToFlashStringHelper(), ps2.ToFlashStringHelper());
  EXPECT_NE(ps1, FLASHSTR(nullptr));

  // Test that the fallback to comparing bytes works. Note that this could be
  // undercut if the compiler+linker collapsed strings that are a suffix of
  // another string into that string. We'll work around that by having two
  // different longer strings which ps1 is a suffix of.

  auto fs1 = MCU_FLASHSTR("Xfoobar");
  ProgmemString ps3(fs1);
  EXPECT_EQ(PrintValueToStdString(ps3), "Xfoobar");
  EXPECT_NE(ps1, ps3);

  auto fs1_suffix = FLASHSTR(reinterpret_cast<const char*>(fs1) + 1);
  ProgmemString ps4(fs1_suffix);
  EXPECT_EQ(PrintValueToStdString(ps4), "foobar");
  EXPECT_EQ(ps1, ps4);
  EXPECT_NE(ps1.ToFlashStringHelper(), ps4.ToFlashStringHelper());

  auto fs2 = MCU_FLASHSTR("Yfoobar");
  ProgmemString ps5(fs2);
  EXPECT_EQ(PrintValueToStdString(ps5), "Yfoobar");
  EXPECT_NE(ps1, ps5);

  auto fs2_suffix = FLASHSTR(reinterpret_cast<const char*>(fs2) + 1);
  ProgmemString ps6(fs2_suffix);
  EXPECT_EQ(PrintValueToStdString(ps6), "foobar");
  EXPECT_EQ(ps1, ps6);
  EXPECT_NE(ps1.ToFlashStringHelper(), ps5.ToFlashStringHelper());
}

TEST(ProgmemStringArrayTest, Empty) {
  static constexpr ProgmemStringArray empty1;
  static constexpr ProgmemStringArray empty2(empty1);
  static constexpr ProgmemStringArray empty3 = empty1;
  static constexpr ProgmemStringArray empty4 = {};

  EXPECT_EQ(empty1.size, 0);
  EXPECT_EQ(empty1.array, nullptr);

  EXPECT_EQ(empty2.size, 0);
  EXPECT_EQ(empty2.array, nullptr);

  EXPECT_EQ(empty3.size, 0);
  EXPECT_EQ(empty3.array, nullptr);

  EXPECT_EQ(empty4.size, 0);
  EXPECT_EQ(empty4.array, nullptr);
}

TEST(ProgmemStringArrayTest, NonEmpty) {
  static constexpr ProgmemString str1(MCU_PSD("str1"));
  static constexpr ProgmemString str2(MCU_PSD("str2"));
  static constexpr ProgmemString raw_array[] = {str1, str2};
  static constexpr ProgmemStringArray a1(raw_array);
  static constexpr ProgmemStringArray a2(a1);
  static constexpr ProgmemStringArray a3 = a2;
  //  static constexpr ProgmemStringArray a4 = ProgmemStringArray{{str2, str1}};
  /* The preceding fails to compile with these errors:

progmem_string_test.cc:106:39: error: constexpr variable 'a4' must be
initialized by a constant expression
  static constexpr ProgmemStringArray a4 = ProgmemStringArray{{str2, str1}};
                                      ^    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
progmem_string_test.cc:106:39: note: pointer to subobject of temporary is not a
constant expression
progmem_string_test.cc:106:63: note: temporary created here
  static constexpr ProgmemStringArray a4 = ProgmemStringArray{{str2, str1}};
                                                              ^
  */
  // I wanted to get something like this working so as to NOT require that a
  // separate array (ala raw_array above) be define by the user.

  EXPECT_EQ(a1.size, 2);
  EXPECT_EQ(a1.array, raw_array);

  EXPECT_EQ(a2.size, a1.size);
  EXPECT_EQ(a2.array, a1.array);

  EXPECT_EQ(a3.size, a1.size);
  EXPECT_EQ(a3.array, a1.array);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
