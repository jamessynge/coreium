// This contains tests of progmem_string_data.h, and also non-tests which print
// info about the types produced by the macros and function templates, thus
// aiding the development of those.

#include "progmem_string_data.h"

#include <cstddef>
#include <ostream>
#include <regex>  // NOLINT
#include <string>
#include <string_view>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/test_strings.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"
#include "o_print_stream.h"
#include "progmem_string_view.h"

namespace mcucore {
namespace progmem_string_data {
namespace test {
namespace {
using ::testing::StartsWith;

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
  static std::regex ns_re("(::)?mcucore::progmem_string_data::");  // NOLINT
  return std::regex_replace(pretty_type_name, ns_re, "");
}

TEST(ProgmemStringDataTest, ExplicitProgmemStrData) {
  using Type = ProgmemStringData<'H', 'E', 'L', 'L', 'O'>;
  auto printable = MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 5);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 5);
  EXPECT_EQ(out.str(), "HELLO");
}

TEST(ProgmemStringDataTest, EmptyProgmemStrData) {
  using Type = ProgmemStringData<>;
  auto printable = MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 0);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(ProgmemStringDataTest, Phase1StringFragmentHasNulls) {
  // When we don't search for the null character at the end, the template
  // class's parameter pack is padded out to have 16 characters.
  using Type = _PSD_PHASE1_TYPE(, "Hello!");
  using ExpectedType =
      Phase1StringFragment<false, 10, false, 'H', 'e', 'l', 'l', 'o', '!', '\0',
                           '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                           '\0'>;
  // We add "extra" parentheses around the value passed to the EXPECT_TRUE macro
  // because otherwise the preprocessor interprets the comma as separating two
  // arguments to EXPECT_TRUE.
  EXPECT_TRUE((std::is_same<Type, ExpectedType>::value))
      << "\n         Type: " << PrettyTypeName<Type>()
      << "\n ExpectedType: " << PrettyTypeName<ExpectedType>();
}

template <typename PSD>
void ValidateProgmemStrData(std::string_view sv, std::string_view expected,
                            const int lineno) {
  VLOG(1) << "len(sv)=" << sv.size()
          << (sv.size() > expected.size() ? " (truncated), sv: " : ", sv: ")
          << sv << std::endl;
  VLOG(1) << "PSD: " << PrettyTypeName<PSD>() << std::endl;
  EXPECT_EQ((sizeof PSD::kData) - 1, expected.size()) << "Lineno=" << lineno;
  EXPECT_EQ(PSD::kData[sizeof PSD::kData - 1], '\0') << "Lineno=" << lineno;
  auto actual = std::string_view(PSD::kData, sizeof PSD::kData - 1);
  EXPECT_EQ(actual, expected) << "Lineno=" << lineno;
}

TEST(ProgmemStringDataTest, ValidateProgmemStrData_32) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x, y)                                                       \
  {                                                                            \
    using PSD = decltype(ProvideStorage(                                       \
        LengthCheck<true>(), _PSD_CONCAT_32_TYPE(_PSD_STRFRAG_TYPE, 0, x)())); \
    ValidateProgmemStrData<PSD>(x, y, __LINE__);                               \
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

TEST(ProgmemStringDataTest, ValidateProgmemStrData_64) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x, y)                                                       \
  {                                                                            \
    using PSS = decltype(ProvideStorage(                                       \
        LengthCheck<true>(), _PSD_CONCAT_64_TYPE(_PSD_STRFRAG_TYPE, 0, x)())); \
    ValidateProgmemStrData<PSS>(x, y, __LINE__);                               \
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

TEST(ProgmemStringDataTest, ValidateProgmemStrData_128) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x, y)                                   \
  {                                                        \
    using PSS = decltype(ProvideStorage(                   \
        LengthCheck<true>(),                               \
        _PSD_CONCAT_128_TYPE(_PSD_STRFRAG_TYPE, 0, x)())); \
    ValidateProgmemStrData<PSS>(x, y, __LINE__);           \
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

TEST(ProgmemStringDataTest, ValidateProgmemStrData_256) {
#undef MAKE_BLOCK
#define MAKE_BLOCK(x, y)                                   \
  {                                                        \
    using PSS = decltype(ProvideStorage(                   \
        LengthCheck<true>(),                               \
        _PSD_CONCAT_256_TYPE(_PSD_STRFRAG_TYPE, 0, x)())); \
    ValidateProgmemStrData<PSS>(x, y, __LINE__);           \
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

TEST(ProgmemStringDataTest, PrintMcuFlashstr) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(MCU_FLASHSTR("Echo, echo, echo, echo, echo")), 28);
  EXPECT_EQ(out.str(), "Echo, echo, echo, echo, echo");
}

TEST(ProgmemStringDataTest, PrintEmptyMcuFlashstr) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(MCU_FLASHSTR("")), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(ProgmemStringDataTest, StreamMcuFlashstr) {
  mcucore::test::PrintToStdString out;
  OPrintStream strm(out);
  strm << MCU_FLASHSTR("foo, Bar, BAZ");
  EXPECT_EQ(out.str(), "foo, Bar, BAZ");
}

TEST(ProgmemStringDataTest, EmbeddedNullsAreIncludedInProgmem) {
  using Type = _PSD_TYPE_128(_PSD_STRFRAG_TYPE, "\0abc\0");
  EXPECT_EQ(6, sizeof(Type::kData));
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(MCU_FLASHSTR_128("\0abc\0")), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(ProgmemStringDataTest, McuPsvPrintTo) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(MCU_PSV("Hey There").printTo(out), 9);
  EXPECT_EQ(out.str(), "Hey There");
  EXPECT_EQ(MCU_PSV("Hey There").size(), 9);
}

TEST(ProgmemStringDataTest, StreamMcuPsv) {
  mcucore::test::PrintToStdString out;
  OPrintStream strm(out);
  strm << MCU_PSV("Hey There");
  EXPECT_EQ(out.str(), "Hey There");
}

TEST(ProgmemStringDataTest, McuPsvToProgmemStringView) {
  ProgmemStringView progmem_string_view = MCU_PSV("Hey There");
  EXPECT_EQ(progmem_string_view.size(), 9);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(progmem_string_view.printTo(out), 9);
  EXPECT_EQ(out.str(), "Hey There");
}

TEST(ProgmemStringDataTest, StreamMcuLit) {
  auto literal = MCU_LIT("Echo, echo, etc");
  VLOG(1) << "decltype(literal): " << PrettyTypeName<decltype(literal)>()
          << std::endl;
  EXPECT_EQ(literal.size(), 15);
  mcucore::test::PrintToStdString out;
  OPrintStream strm(out);
  strm << MCU_LIT("Echo, echo, etc");
  EXPECT_EQ(out.str(), "Echo, echo, etc");
}

TEST(ProgmemStringDataTest, StringLiteralIsTooLong) {
  // Test that when too long a string is used, the type deduction produces the
  // type StringLiteralIsTooLong, rather than a ProgmemStringData.

#define LENGTH_OK_TEST(len, max)                                  \
  {                                                               \
    static_assert(len <= max, "len too high!");                   \
    using T = _PSD_TYPE_##max(_PSD_STRFRAG_TYPE, TEST_STR_##len); \
    static_assert((sizeof(T::kData) - 1) == len);                 \
  }

#define BAD_LENGTH_TEST(len, max)                                 \
  {                                                               \
    using T = _PSD_TYPE_##max(_PSD_STRFRAG_TYPE, TEST_STR_##len); \
    static_assert(std::is_same<T, StringLiteralIsTooLong>::value, \
                  "Should not be valid.");                        \
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

TEST(ProgmemStringDataTest, BasenameAfterForwardSlashes) {
  EXPECT_EQ(PrintValueToStdString(MCU_BASENAME_32("/" TEST_STR_18 "/abc.def")),
            "abc.def");
  EXPECT_EQ(PrintValueToStdString(
                MCU_BASENAME_64(TEST_STR_32 "/" TEST_STR_18 "/ABC.def")),
            "ABC.def");
  EXPECT_EQ(PrintValueToStdString(MCU_BASENAME_128(
                TEST_STR_64 "/" TEST_STR_32 "/" TEST_STR_17 "/abc.c")),
            "abc.c");
  EXPECT_EQ(PrintValueToStdString(
                MCU_BASENAME_256(TEST_STR_128 "/" TEST_STR_64 "/" TEST_STR_32
                                              "/" TEST_STR_17 "/abC.def")),
            "abC.def");
  EXPECT_EQ(PrintValueToStdString(MCU_BASENAME_512(
                TEST_STR_256 "/" TEST_STR_128 "/" TEST_STR_64 "/" TEST_STR_32
                             "/" TEST_STR_16 "/ABC.DEF")),
            "ABC.DEF");
  EXPECT_EQ(PrintValueToStdString(MCU_BASENAME_1024(
                TEST_STR_512 "/" TEST_STR_256 "/" TEST_STR_128 "/" TEST_STR_64
                             "/" TEST_STR_32 "/" TEST_STR_16 "/abc.DEF")),
            "abc.DEF");
}

TEST(ProgmemStringDataTest, BasenameAfterBackwardSlashes) {
  EXPECT_EQ(
      PrintValueToStdString(MCU_BASENAME_32("\\" TEST_STR_18 "\\abc.def")),
      "abc.def");
  EXPECT_EQ(PrintValueToStdString(
                MCU_BASENAME_64(TEST_STR_32 "\\" TEST_STR_18 "\\ABC.def")),
            "ABC.def");
  EXPECT_EQ(PrintValueToStdString(MCU_BASENAME_128(
                TEST_STR_64 "\\" TEST_STR_32 "\\" TEST_STR_17 "\\abc.c")),
            "abc.c");
  EXPECT_EQ(PrintValueToStdString(
                MCU_BASENAME_256(TEST_STR_128 "\\" TEST_STR_64 "\\" TEST_STR_32
                                              "\\" TEST_STR_17 "\\abC.def")),
            "abC.def");
  EXPECT_EQ(PrintValueToStdString(MCU_BASENAME_512(
                TEST_STR_256 "\\" TEST_STR_128 "\\" TEST_STR_64 "\\" TEST_STR_32
                             "\\" TEST_STR_16 "\\ABC.DEF")),
            "ABC.DEF");
  EXPECT_EQ(
      PrintValueToStdString(MCU_BASENAME_1024(
          TEST_STR_512 "\\" TEST_STR_256 "\\" TEST_STR_128 "\\" TEST_STR_64
                       "\\" TEST_STR_32 "\\" TEST_STR_16 "\\abc.DEF")),
      "abc.DEF");
}

TEST(ProgmemStringDataTest, NoSlash) {
  mcucore::test::PrintToStdString out;
  out.print(MCU_BASENAME("foo.bar.baz"));
  EXPECT_EQ(out.str(), "foo.bar.baz");
}

TEST(ProgmemStringDataTest, LeadingSlash) {
  mcucore::test::PrintToStdString out;
  out.print(MCU_BASENAME("/bar.baz"));
  EXPECT_EQ(out.str(), "bar.baz");
}

TEST(ProgmemStringDataTest, LeadingSlashes) {
  mcucore::test::PrintToStdString out;
  out.print(MCU_BASENAME("//bar.baz"));
  EXPECT_EQ(out.str(), "bar.baz");
}

TEST(ProgmemStringDataTest, MiddleSlash) {
  mcucore::test::PrintToStdString out;
  out.print(MCU_BASENAME("foo/bar.baz"));
  EXPECT_EQ(out.str(), "bar.baz");
}

TEST(ProgmemStringDataTest, LeadingAndMiddleSlashes) {
  mcucore::test::PrintToStdString out;
  out.print(MCU_BASENAME("//foo//bar/baz.cc"));
  EXPECT_EQ(out.str(), "baz.cc");
}

TEST(ProgmemStringDataTest, TrailingSlash) {
  mcucore::test::PrintToStdString out;
  out.print(MCU_BASENAME("foo.bar.baz/"));
  EXPECT_EQ(out.str(), "");
}

TEST(ProgmemStringDataTest, Basename1024_ThisFile) {
  VLOG(1) << "__FILE__: " << __FILE__ << std::endl;
  VLOG(1) << "MCU_BASENAME_1024(__FILE__): "
          << PrintValueToStdString(MCU_BASENAME_1024(__FILE__)) << std::endl;
  EXPECT_THAT(PrintValueToStdString(MCU_BASENAME_1024(__FILE__)),
              StartsWith("progmem_string_data_test.c"));
}

// A 512 character path. This rather odd set of characters is chosen so that no
// two leaf StringFragments will be the same as any other in the path, thus
// ensuring the maximum amount of "work" by the compiler.
#define A_REALLY_LONG_PATH                                                     \
  "/23456789a123456789/123456789c123456789/123456789e123456789/123456789/"     \
  "123456789/123456789i123456789.123456789j123456789/"                         \
  "123456789l123456789m123456789n123456789o123456789/"                         \
  "123456789q123456789r123456789."                                             \
  "123456789s123456789t123456789u123456789v123456789w123456789x123456789y1234" \
  "56789z123456789A123456789."                                                 \
  "123456789B123456789C123456789D123456789E123456789F123456789G123456789H1234" \
  "56789I123456789J123456789."                                                 \
  "123456789K123456789L123456789M123456789N123456789O123456789P123456789Q1234" \
  "56789R123456789S123456789/123456789T.2"

#define BASENAME_512_1                                                   \
  EXPECT_EQ(PrintValueToStdString(MCU_BASENAME_512(A_REALLY_LONG_PATH)), \
            "123456789T.2")
#define BASENAME_512_2 \
  BASENAME_512_1;      \
  BASENAME_512_1
#define BASENAME_512_4 \
  BASENAME_512_2;      \
  BASENAME_512_2
#define BASENAME_512_8 \
  BASENAME_512_4;      \
  BASENAME_512_4
#define BASENAME_512_16 \
  BASENAME_512_8;       \
  BASENAME_512_8
#define BASENAME_512_32 \
  BASENAME_512_16;      \
  BASENAME_512_16
#define BASENAME_512_64 \
  BASENAME_512_32;      \
  BASENAME_512_32
#define BASENAME_512_128 \
  BASENAME_512_64;       \
  BASENAME_512_64
#define BASENAME_512_256 \
  BASENAME_512_128;      \
  BASENAME_512_128

TEST(ProgmemStringDataTest, Basename512) {
  VLOG(1) << "A_REALLY_LONG_PATH: " << A_REALLY_LONG_PATH << std::endl;
  EXPECT_EQ(std::string_view(A_REALLY_LONG_PATH).size(), 512);

  // We want to be able to have many separate evaluations of MCU_BASENAME_512 so
  // that we can see whether it has a strongly negative impact on compiler
  // performance.

  BASENAME_512_1;
}

// Tests that each power-of-two maximum size compiles and works.

#define COMPILES_TEST(x, len, max)                                          \
  TEST(ProgmemStringDataTest, LiteralOfLength##len##CompilesWithMax##max) { \
    VLOG(1) << "len(x)=" << ((sizeof x) - 1) << ", x: " << x << std::endl;  \
    using Type = _PSD_TYPE_##max(_PSD_STRFRAG_TYPE, x);                     \
    EXPECT_EQ(len + 1, sizeof(Type::kData));                                \
    mcucore::test::PrintToStdString out;                                    \
    EXPECT_EQ(out.print(MCU_FLASHSTR_##max(x)), len);                       \
    EXPECT_EQ(out.str(), x);                                                \
    EXPECT_EQ(PrintValueToStdString(MCU_BASENAME_##max(x)), x);             \
  }

COMPILES_TEST(TEST_STR_32, 32, 32);
COMPILES_TEST(TEST_STR_64, 64, 64);
COMPILES_TEST(TEST_STR_128, 128, 128);
COMPILES_TEST(TEST_STR_256, 256, 256);
COMPILES_TEST(TEST_STR_512, 512, 512);
COMPILES_TEST(TEST_STR_1024, 1024, 1024);

}  // namespace
}  // namespace test
}  // namespace progmem_string_data
}  // namespace mcucore
