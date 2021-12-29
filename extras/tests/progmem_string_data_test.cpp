// This contains tests of progmem_string_data.h, and also non-tests which print
// info about the types produced by the macros and function templates, thus
// aiding the development of those.

#include "progmem_string_data.h"

#include <cstddef>
#include <ostream>
#include <regex>  // NOLINT
#include <string>
#include <string_view>

#include "extras/test_tools/print_value_to_std_string.h"

// Prevent inclusion of inline_literal.h when testing progmem_string_data.h.
#define MCUCORE_SRC_INLINE_LITERAL_H_

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/test_strings.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "literal.h"
#include "mcucore_platform.h"
#include "o_print_stream.h"
#include "progmem_string_view.h"

namespace mcucore {
namespace progmem_data {
namespace test {
namespace {
using ::testing::HasSubstr;

TEST(GenerateTestStringsTest, DISABLED_GenerateThreeStrings) {
  for (size_t size2 : {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192}) {
    size_t size1 = size2 - 1, size3 = size2 + 1;
    std::string s3 = ::mcucore::test::GenerateTestString(size3);
    std::string s2 = s3.substr(0, size2);
    std::string s1 = s3.substr(0, size1);

    std::cout << "#define TEST_STR_" << size1 << " \"" << s1 << "\""
              << std::endl;
    std::cout << "#define TEST_STR_" << size2 << " TEST_STR_" << size1 << " \""
              << s2.substr(size1) << "\"" << std::endl;
    std::cout << "#define TEST_STR_" << size3 << " TEST_STR_" << size2 << " \""
              << s3.substr(size2) << "\"" << std::endl
              << std::endl;
  }
}

template <typename T>
constexpr std::string_view PrettyFunctionName() {
#if defined(__clang__) || defined(__GNUC__)
  return std::string_view{__PRETTY_FUNCTION__};
#elif defined(__MSC_VER)
  return std::string_view{__FUNCSIG__};
#else
#error Unsupported compiler
#endif
}

// Outside of the template so its computed once
struct PrettyFunctionNameOffsets {
  static constexpr auto kSentinalName = PrettyFunctionName<double>();
  static constexpr auto kPrefixSize = kSentinalName.find("double");
  static constexpr auto kSuffixSize =
      kSentinalName.size() - kSentinalName.rfind("double") - 6;
};

template <typename T>
std::string PrettyTypeName() {
  auto this_function_name = PrettyFunctionName<T>();
  auto without_prefix =
      this_function_name.substr(PrettyFunctionNameOffsets::kPrefixSize);
  std::string pretty_type_name(without_prefix.substr(
      0, without_prefix.size() - PrettyFunctionNameOffsets::kSuffixSize));
  static std::regex null_char_re("'\\\\x00'");  // NOLINT
  pretty_type_name = std::regex_replace(pretty_type_name, null_char_re, "NUL");
  static std::regex ns_re("(::)?mcucore::progmem_data::");  // NOLINT
  return std::regex_replace(pretty_type_name, ns_re, "");
}

TEST(DiscardAfterLiteralTest, FirstFewFragments) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using RAW = _PSD_PHASE1_TYPE(n, x);                                  \
    std::cout << "R: " << PrettyTypeName<RAW>() << std::endl;            \
    using DISCARDED = _PSD_FRAGMENT_TYPE(n, x);                          \
    std::cout << "D: " << PrettyTypeName<DISCARDED>() << std::endl       \
              << std::endl;                                              \
  }

  MAKE_BLOCK(0, "");
  MAKE_BLOCK(1, "");

  MAKE_BLOCK(0, "1");
  MAKE_BLOCK(1, "1");

  MAKE_BLOCK(0, "12");
  MAKE_BLOCK(1, "12");

  MAKE_BLOCK(0, TEST_STR_14);
  MAKE_BLOCK(1, TEST_STR_14);

  MAKE_BLOCK(0, TEST_STR_15);
  MAKE_BLOCK(1, TEST_STR_15);

  MAKE_BLOCK(0, TEST_STR_16);
  MAKE_BLOCK(1, TEST_STR_16);

  MAKE_BLOCK(0, TEST_STR_17);
  MAKE_BLOCK(1, TEST_STR_17);

  MAKE_BLOCK(0, TEST_STR_31);
  MAKE_BLOCK(1, TEST_STR_31);
  MAKE_BLOCK(2, TEST_STR_31);

  MAKE_BLOCK(0, TEST_STR_32);
  MAKE_BLOCK(1, TEST_STR_32);
  MAKE_BLOCK(2, TEST_STR_32);

  MAKE_BLOCK(0, TEST_STR_33);
  MAKE_BLOCK(1, TEST_STR_33);
  MAKE_BLOCK(2, TEST_STR_33);
}

TEST(ConcatTest, NonMacroCases) {
  {
    using T1 = StringFragment<'a'>;
    std::cout << "T21: " << PrettyTypeName<T1>() << std::endl;

    using T2 = StringFragment<'b'>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
    EXPECT_THAT(PrettyTypeName<C>(), HasSubstr("<'a', 'b'>"));
  }
  {
    using T1 = StringFragment<'a', 'c'>;
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = StringFragment<>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
    EXPECT_THAT(PrettyTypeName<C>(), HasSubstr("<'a', 'c'>"));
  }
  {
    using T1 = StringFragment<>;
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = StringFragment<>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
    EXPECT_THAT(PrettyTypeName<C>(), HasSubstr("<>"));
  }
  // Degenerate cases that shouldn't happen, but which we don't treat as
  // invalid.
  {
    using T1 = StringFragment<>;
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = StringFragment<'B', 'C'>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
    EXPECT_THAT(PrettyTypeName<C>(), HasSubstr("<'B', 'C'>"));
  }
}

TEST(ConcatTest, DecomposedMacro32Cases) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x)                                                        \
  {                                                                          \
    std::cout << "len(x)=" << ((sizeof x) - 1) << ", x: " << x << std::endl; \
    using T0 = _PSD_FRAGMENT_TYPE(0, x);                                     \
    std::cout << "T0: " << PrettyTypeName<T0>() << std::endl;                \
    using T1 = _PSD_FRAGMENT_TYPE(1, x);                                     \
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;                \
    using CT = _PSD_CONCAT_TYPE(T0, T1);                                     \
    std::cout << "CT: " << PrettyTypeName<CT>() << std::endl;                \
    using CFT = _PSD_CONCAT_FRAGMENTS_TYPE(, 0, 1, x);                       \
    std::cout << "CFT: " << PrettyTypeName<CFT>() << std::endl << std::endl; \
  }

  MAKE_BLOCK("");
  MAKE_BLOCK("1");
  MAKE_BLOCK("12");
  MAKE_BLOCK(TEST_STR_14);
  MAKE_BLOCK(TEST_STR_15);
  MAKE_BLOCK(TEST_STR_16);
  MAKE_BLOCK(TEST_STR_17);
  MAKE_BLOCK(TEST_STR_31);
  MAKE_BLOCK(TEST_STR_32);
  MAKE_BLOCK(TEST_STR_33);
}

TEST(ConcatTest, MacroCases) {
  {
#define S "a"
    std::cout << "S: " << S << std::endl;

    using T1 = _PSD_FRAGMENT_TYPE(0, S);
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = _PSD_FRAGMENT_TYPE(1, S);
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = _PSD_CONCAT_TYPE(T1, T2);
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl;
#undef S
  }
  {
#define S "a"
    std::cout << "S: " << S << std::endl;

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmno"
    std::cout << "S: " << S << std::endl;

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnop"
    std::cout << "S: " << S << std::endl;

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopq"
    std::cout << "S: " << S << std::endl;

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz01234"
    std::cout << "S: " << S << std::endl;

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz012345"
    std::cout << "S: " << S << std::endl;

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz0123456"
    std::cout << "S: " << S << std::endl;

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz0123456"
    std::cout << "S: " << S << std::endl;

#define A 0
#define B 1

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, A, B, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
#undef A
#undef B
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz0123456"
    std::cout << "S: " << S << std::endl;

#define A 1
#define B 2

    using T = _PSD_CONCAT_FRAGMENTS_TYPE(0, A, B, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
#undef A
#undef B
  }
}

TEST(ConcatTest, Concat32MacroCases) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _PSD_CONCAT_32_TYPE(n, x);                                 \
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl; \
  }

  MAKE_BLOCK(, "");
  MAKE_BLOCK(0, "");

  MAKE_BLOCK(0, "1");
  MAKE_BLOCK(1, "1");

  MAKE_BLOCK(0, "12");
  MAKE_BLOCK(1, "12");

  MAKE_BLOCK(0, TEST_STR_14);
  MAKE_BLOCK(1, TEST_STR_14);

  MAKE_BLOCK(0, TEST_STR_15);
  MAKE_BLOCK(1, TEST_STR_15);

  MAKE_BLOCK(0, TEST_STR_16);
  MAKE_BLOCK(1, TEST_STR_16);

  MAKE_BLOCK(0, TEST_STR_17);
  MAKE_BLOCK(1, TEST_STR_17);

  MAKE_BLOCK(0, "abcdefghijklmnop");
  MAKE_BLOCK(1, "abcdefghijklmnopq");

  MAKE_BLOCK(0, "abcdefghijklmnopqrstuvwxyz01234");
  MAKE_BLOCK(1, "abcdefghijklmnopqrstuvwxyz01234");

  MAKE_BLOCK(0, "abcdefghijklmnopqrstuvwxyz012345");
  MAKE_BLOCK(1, "abcdefghijklmnopqrstuvwxyz012345");

  MAKE_BLOCK(0, "abcdefghijklmnopqrstuvwxyz0123456");
  MAKE_BLOCK(1, "abcdefghijklmnopqrstuvwxyz0123456");
}

TEST(ConcatTest, Concat64MacroCases) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _PSD_CONCAT_64_TYPE(n, x);                                 \
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl; \
  }

  MAKE_BLOCK(, "");
  MAKE_BLOCK(, "a");

  MAKE_BLOCK(0, TEST_STR_31);
  MAKE_BLOCK(0, TEST_STR_32);
  MAKE_BLOCK(0, TEST_STR_33);
  MAKE_BLOCK(0, TEST_STR_63);
  MAKE_BLOCK(0, TEST_STR_64);
  MAKE_BLOCK(0, TEST_STR_65);
  MAKE_BLOCK(0, TEST_STR_127);
  MAKE_BLOCK(0, TEST_STR_128);
  MAKE_BLOCK(0, TEST_STR_129);

  MAKE_BLOCK(1, TEST_STR_129);
  MAKE_BLOCK(1, TEST_STR_257);
  MAKE_BLOCK(1, TEST_STR_511);
}

TEST(ConcatTest, Concat128MacroCases) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _PSD_CONCAT_128_TYPE(n, x);                                \
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl; \
  }

  MAKE_BLOCK(0, "");
  MAKE_BLOCK(0, "a");
  MAKE_BLOCK(0, TEST_STR_31);
  MAKE_BLOCK(0, TEST_STR_32);
  MAKE_BLOCK(0, TEST_STR_33);
  MAKE_BLOCK(0, TEST_STR_63);
  MAKE_BLOCK(0, TEST_STR_64);
  MAKE_BLOCK(0, TEST_STR_65);
  MAKE_BLOCK(0, TEST_STR_127);
  MAKE_BLOCK(0, TEST_STR_128);
  MAKE_BLOCK(0, TEST_STR_129);

  MAKE_BLOCK(1, TEST_STR_129);
  MAKE_BLOCK(1, TEST_STR_257);
  MAKE_BLOCK(1, TEST_STR_511);
}

TEST(ConcatTest, Concat256MacroCases) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _PSD_CONCAT_256_TYPE(n, x);                                \
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl; \
  }
  MAKE_BLOCK(0, "");
  MAKE_BLOCK(0, "a");
  MAKE_BLOCK(0, TEST_STR_31);
  MAKE_BLOCK(0, TEST_STR_33);
  MAKE_BLOCK(0, TEST_STR_65);
  MAKE_BLOCK(0, TEST_STR_129);
  MAKE_BLOCK(0, TEST_STR_255);
  MAKE_BLOCK(0, TEST_STR_256);
  MAKE_BLOCK(0, TEST_STR_257);
  MAKE_BLOCK(0, TEST_STR_513);
  MAKE_BLOCK(0, TEST_STR_1025);
  MAKE_BLOCK(0, TEST_STR_2049);
}

////////////////////////////////////////////////////////////////////////////////
// Tests of the upper limits of string literal length.

TEST(ConcatTest, Concat512MacroCases) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _PSD_CONCAT_512_TYPE(n, x);                                \
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl; \
  }
  MAKE_BLOCK(, "");
  MAKE_BLOCK(0, TEST_STR_257);
  MAKE_BLOCK(0, TEST_STR_513);
  MAKE_BLOCK(0, TEST_STR_1025);
  MAKE_BLOCK(0, TEST_STR_2049);

  MAKE_BLOCK(1, TEST_STR_4096);
  MAKE_BLOCK(1, TEST_STR_4097);
  MAKE_BLOCK(1, TEST_STR_8191);
  MAKE_BLOCK(2, TEST_STR_8193);
}

TEST(ConcatTest, Concat1024MacroCases) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _PSD_CONCAT_1024_TYPE(n, x);                               \
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl; \
  }
  MAKE_BLOCK(, "");
  MAKE_BLOCK(0, TEST_STR_257);
  MAKE_BLOCK(0, TEST_STR_513);
  MAKE_BLOCK(0, TEST_STR_1023);
  MAKE_BLOCK(0, TEST_STR_1024);
  MAKE_BLOCK(0, TEST_STR_1025);
  MAKE_BLOCK(0, TEST_STR_2049);

  MAKE_BLOCK(1, TEST_STR_1025);
  MAKE_BLOCK(1, TEST_STR_2049);
  MAKE_BLOCK(1, TEST_STR_4096);
  MAKE_BLOCK(1, TEST_STR_4097);
  MAKE_BLOCK(1, TEST_STR_8191);
  MAKE_BLOCK(2, TEST_STR_8193);
}

// #define COMPILES_TEST(x, len)                                            \
//   TEST(ConcatTest, LiteralOfLength##len##Compiles) {                     \
//     std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
//               << std::endl;                                              \
//     using Type = _PSD_TYPE_##max(x);                                 \
//     EXPECT_EQ(len + 1, sizeof(Type::kData));                             \
//     mcucore::test::PrintToStdString out;                                 \
//     EXPECT_EQ(out.print(MCU_FLASHSTR_##max(x)), len);                    \
//     EXPECT_EQ(out.str(), x);                                             \
//   }

// COMPILES_TEST(TEST_STR_31, 31, 32);
// COMPILES_TEST(TEST_STR_63, 63, 64);
// COMPILES_TEST(TEST_STR_127, 127, 128);
// COMPILES_TEST(TEST_STR_255, 255, 256);
// COMPILES_TEST(TEST_STR_511, 511, 512);

TEST(InlineLiteralTest, ExplicitProgmemStrData) {
  using Type = ProgmemStringData<'H', 'E', 'L', 'L', 'O'>;
  auto printable = MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 5);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 5);
  EXPECT_EQ(out.str(), "HELLO");
}

TEST(InlineLiteralTest, EmptyProgmemStrData) {
  using Type = ProgmemStringData<>;
  auto printable = MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 0);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(InlineLiteralTest, Phase1StringFragmentHasNulls) {
  // When we don't search for the null character at the end, the template
  // class's parameter pack is padded out to have 16 characters.
  using Type = _PSD_PHASE1_TYPE(, "Hello!");
  EXPECT_THAT(PrettyTypeName<Type>(),
              testing::HasSubstr("'H', 'e', 'l', 'l', 'o', '!', NUL, NUL, NUL, "
                                 "NUL, NUL, NUL, NUL, NUL, NUL, NUL"));
}

template <typename PSD>
void ValidateProgmemStrData(std::string_view sv, std::string_view expected,
                            const int lineno) {
  std::cout << "len(sv)=" << sv.size()
            << (sv.size() > expected.size() ? " (truncated), sv: " : ", sv: ")
            << sv << std::endl;
  std::cout << "PSD: " << PrettyTypeName<PSD>() << std::endl;
  EXPECT_EQ((sizeof PSD::kData) - 1, expected.size()) << "Lineno=" << lineno;
  EXPECT_EQ(PSD::kData[sizeof PSD::kData - 1], '\0') << "Lineno=" << lineno;
  auto actual = std::string_view(PSD::kData, sizeof PSD::kData - 1);
  EXPECT_EQ(actual, expected) << "Lineno=" << lineno;
}

TEST(InlineLiteralTest, ValidateProgmemStrData_32) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x, y)                                          \
  {                                                               \
    using PSD = decltype(::mcucore::progmem_data::ProvideStorage( \
        LengthCheck<true>(), _PSD_CONCAT_32_TYPE(, x)()));        \
    ValidateProgmemStrData<PSD>(x, y, __LINE__);                  \
  }

  MAKE_BLOCK("", "");
  MAKE_BLOCK("1", "1");
  MAKE_BLOCK("12", "12");
  MAKE_BLOCK("123", "123");
  MAKE_BLOCK(TEST_STR_14, TEST_STR_14);
  MAKE_BLOCK(TEST_STR_15, TEST_STR_15);
  MAKE_BLOCK(TEST_STR_16, TEST_STR_16);
  MAKE_BLOCK(TEST_STR_17, TEST_STR_17);

  MAKE_BLOCK(TEST_STR_31, TEST_STR_31);
  MAKE_BLOCK(TEST_STR_32, TEST_STR_32);
  MAKE_BLOCK(TEST_STR_33, TEST_STR_32);  // Note: truncated.
}

TEST(InlineLiteralTest, ValidateProgmemStrData_64) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x, y)                                          \
  {                                                               \
    using PSS = decltype(::mcucore::progmem_data::ProvideStorage( \
        LengthCheck<true>(), _PSD_CONCAT_64_TYPE(, x)()));        \
    ValidateProgmemStrData<PSS>(x, y, __LINE__);                  \
  }

  MAKE_BLOCK("", "");
  MAKE_BLOCK("a", "a");
  MAKE_BLOCK(TEST_STR_31, TEST_STR_31);
  MAKE_BLOCK(TEST_STR_32, TEST_STR_32);
  MAKE_BLOCK(TEST_STR_33, TEST_STR_33);
  MAKE_BLOCK(TEST_STR_63, TEST_STR_63);
  MAKE_BLOCK(TEST_STR_64, TEST_STR_64);
  MAKE_BLOCK(TEST_STR_65, TEST_STR_64);  // Note: truncated.
}

TEST(InlineLiteralTest, ValidateProgmemStrData_128) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x, y)                                          \
  {                                                               \
    using PSS = decltype(::mcucore::progmem_data::ProvideStorage( \
        LengthCheck<true>(), _PSD_CONCAT_128_TYPE(, x)()));       \
    ValidateProgmemStrData<PSS>(x, y, __LINE__);                  \
  }

  MAKE_BLOCK("", "");
  MAKE_BLOCK("a", "a");
  MAKE_BLOCK(TEST_STR_31, TEST_STR_31);
  MAKE_BLOCK(TEST_STR_32, TEST_STR_32);
  MAKE_BLOCK(TEST_STR_33, TEST_STR_33);
  MAKE_BLOCK(TEST_STR_63, TEST_STR_63);
  MAKE_BLOCK(TEST_STR_64, TEST_STR_64);
  MAKE_BLOCK(TEST_STR_65, TEST_STR_65);
  MAKE_BLOCK(TEST_STR_127, TEST_STR_127);
  MAKE_BLOCK(TEST_STR_128, TEST_STR_128);
  MAKE_BLOCK(TEST_STR_129, TEST_STR_128);
}

TEST(InlineLiteralTest, ValidateProgmemStrData_256) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x, y)                                          \
  {                                                               \
    using PSS = decltype(::mcucore::progmem_data::ProvideStorage( \
        LengthCheck<true>(), _PSD_CONCAT_256_TYPE(, x)()));       \
    ValidateProgmemStrData<PSS>(x, y, __LINE__);                  \
  }

  MAKE_BLOCK("", "");
  MAKE_BLOCK("a", "a");
  MAKE_BLOCK(TEST_STR_31, TEST_STR_31);
  MAKE_BLOCK(TEST_STR_32, TEST_STR_32);
  MAKE_BLOCK(TEST_STR_33, TEST_STR_33);
  MAKE_BLOCK(TEST_STR_63, TEST_STR_63);
  MAKE_BLOCK(TEST_STR_64, TEST_STR_64);
  MAKE_BLOCK(TEST_STR_65, TEST_STR_65);
  MAKE_BLOCK(TEST_STR_127, TEST_STR_127);
  MAKE_BLOCK(TEST_STR_128, TEST_STR_128);
  MAKE_BLOCK(TEST_STR_129, TEST_STR_129);
  MAKE_BLOCK(TEST_STR_255, TEST_STR_255);
  MAKE_BLOCK(TEST_STR_256, TEST_STR_256);
  MAKE_BLOCK(TEST_STR_257, TEST_STR_256);
}

TEST(InlineLiteralTest, PrintMcuFlashstr) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(MCU_FLASHSTR("Echo, echo, echo, echo, echo")), 28);
  EXPECT_EQ(out.str(), "Echo, echo, echo, echo, echo");
}

TEST(InlineLiteralTest, PrintEmptyMcuFlashstr) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(MCU_FLASHSTR("")), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(InlineLiteralTest, StreamMcuFlashstr) {
  mcucore::test::PrintToStdString out;
  OPrintStream strm(out);
  strm << MCU_FLASHSTR("foo, Bar, BAZ");
  EXPECT_EQ(out.str(), "foo, Bar, BAZ");
}

TEST(InlineLiteralTest, EmbeddedNullsAreIncludedInProgmem) {
  using Type = _PSD_TYPE_128("\0abc\0");
  EXPECT_EQ(6, sizeof(Type::kData));
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(MCU_FLASHSTR_128("\0abc\0")), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(InlineLiteralTest, McuPsvPrintTo) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(MCU_PSV("Hey There").printTo(out), 9);
  EXPECT_EQ(out.str(), "Hey There");
  EXPECT_EQ(MCU_PSV("Hey There").size(), 9);
}

TEST(InlineLiteralTest, StreamMcuPsv) {
  mcucore::test::PrintToStdString out;
  OPrintStream strm(out);
  strm << MCU_PSV("Hey There");
  EXPECT_EQ(out.str(), "Hey There");
}

TEST(InlineLiteralTest, McuPsvToProgmemStringView) {
  ProgmemStringView progmem_string_view = MCU_PSV("Hey There");
  EXPECT_EQ(progmem_string_view.size(), 9);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(progmem_string_view.printTo(out), 9);
  EXPECT_EQ(out.str(), "Hey There");
}

TEST(InlineLiteralTest, McuPsvToLiteral) {
  Literal literal = MCU_PSV("Hey There!");
  EXPECT_EQ(literal.size(), 10);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(literal.printTo(out), 10);
  EXPECT_EQ(out.str(), "Hey There!");
}

TEST(InlineLiteralTest, StreamMcuLit) {
  auto literal = MCU_LIT("Echo, echo, etc");
  std::cout << "decltype(literal): " << PrettyTypeName<decltype(literal)>()
            << std::endl;
  EXPECT_EQ(literal.size(), 15);
  mcucore::test::PrintToStdString out;
  OPrintStream strm(out);
  strm << MCU_LIT("Echo, echo, etc");
  EXPECT_EQ(out.str(), "Echo, echo, etc");
}

TEST(InlineLiteralTest, StringLiteralIsTooLong) {
  // Test that when too long a string is used, the type deduction produces the
  // type StringLiteralIsTooLong, rather than a ProgmemStringData.

#define LENGTH_OK_TEST(len, max)                  \
  {                                               \
    static_assert(len <= max, "len too high!");   \
    using T = _PSD_TYPE_##max(TEST_STR_##len);    \
    static_assert((sizeof(T::kData) - 1) == len); \
  }

#define BAD_LENGTH_TEST(len, max)                                             \
  {                                                                           \
    using T = _PSD_TYPE_##max(TEST_STR_##len);                                \
    static_assert(                                                            \
        std::is_same<T,                                                       \
                     ::mcucore::progmem_data::StringLiteralIsTooLong>::value, \
        "Should not be valid.");                                              \
  }

  LENGTH_OK_TEST(32, 32);
  LENGTH_OK_TEST(64, 64);
  LENGTH_OK_TEST(128, 128);
  LENGTH_OK_TEST(256, 256);
  LENGTH_OK_TEST(512, 512);
  LENGTH_OK_TEST(1024, 1024);

  BAD_LENGTH_TEST(33, 32);
  BAD_LENGTH_TEST(65, 64);
  BAD_LENGTH_TEST(129, 128);
  BAD_LENGTH_TEST(257, 256);
  BAD_LENGTH_TEST(513, 512);
  BAD_LENGTH_TEST(1025, 1024);
}

}  // namespace
}  // namespace test
}  // namespace progmem_data
}  // namespace mcucore
