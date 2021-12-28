#include "inline_literal2.h"

#include <string_view>

#include "extras/test_tools/print_to_std_string.h"
#include "gtest/gtest.h"
#include "literal.h"
#include "o_print_stream.h"
#include "progmem_string_view.h"

namespace mcucore {
namespace test {
namespace {
using ::mcucore::progmem_data::ProgmemStrData;

// Goal: address the exceedingly slow performance of compiling TAS_PSV_1024.
// The macro results in a very long chain of template "expansions". sizeof S,
// where S is a string literal, evaluates to the size of that literal, including
// the trailing NUL. We may be able to use this to reduce the number of steps
// in the chain, in exchange for having more unique templates specializations.
//
// Note that using this approach will skip checking for NUL before the end, and
// thus doesn't help us with detecting string literals that have embedded NULs,
// and such string literals don't work well in situations where we need to treat
// them as C-style NUL terminated strings (i.e. the NUL appears before the end
// of the string.

#define NAMESPACE

class StringLiteralIsTooLong;

constexpr size_t kCharGroupSize = 8;

template <char... C>
struct LiteralStringPack final {};

// First type we use during the expansion. If length_ok is true, then the char
// parameter pack isn't too long for the TAS_EXPAND_nnn macro that was used.
template <bool length_ok, size_t trailing_length, char... C>
struct InitialLiteralStringPack final {};

template <bool length_ok, size_t trailing_length>
struct TotalLengthInfo final {};

template <bool more_than_one_group, size_t trailing_length>
struct RemainderInfo final {};

// End of the expansions. We expect more_than_one_group == 0.
template <bool more_than_one_group, size_t trailing_length, char... C>
auto KeepBeforeEnd(RemainderInfo<more_than_one_group, trailing_length>,
                   LiteralStringPack<C...>) -> LiteralStringPack<C...>;

// Case where there are at least kCharGroupSize characters remaining (here
// assumed to be 8).
template <size_t trailing_length, char C1, char C2, char C3, char C4, char C5,
          char C6, char C7, char C8, char... X, char... Y>
auto KeepBeforeEnd(RemainderInfo<true, trailing_length>,
                   LiteralStringPack<X...>, LiteralStringPack<C1>,
                   LiteralStringPack<C2>, LiteralStringPack<C3>,
                   LiteralStringPack<C4>, LiteralStringPack<C5>,
                   LiteralStringPack<C6>, LiteralStringPack<C7>,
                   LiteralStringPack<C8>, LiteralStringPack<Y>...)
    -> decltype(KeepBeforeEnd(
        RemainderInfo<((trailing_length - kCharGroupSize) >= kCharGroupSize),
                      (trailing_length - kCharGroupSize)>(),
        LiteralStringPack<X..., C1, C2, C3, C4, C5, C6, C7, C8>(),
        LiteralStringPack<Y>()...));

template <size_t trailing_length, char C1, char C2, char C3, char C4, char C5,
          char C6, char C7, char C8, char... Y>
auto KeepBeforeEnd(RemainderInfo<true, trailing_length>, LiteralStringPack<C1>,
                   LiteralStringPack<C2>, LiteralStringPack<C3>,
                   LiteralStringPack<C4>, LiteralStringPack<C5>,
                   LiteralStringPack<C6>, LiteralStringPack<C7>,
                   LiteralStringPack<C8>, LiteralStringPack<Y>...)
    -> decltype(KeepBeforeEnd(
        RemainderInfo<((trailing_length - kCharGroupSize) >= kCharGroupSize),
                      (trailing_length - kCharGroupSize)>(),
        LiteralStringPack<C1, C2, C3, C4, C5, C6, C7, C8>(),
        LiteralStringPack<Y>()...));

// Case where there are between 1 and kCharGroupSize characters remaining.
template <size_t trailing_length, char A, char... X, char... Y>
auto KeepBeforeEnd(RemainderInfo<false, trailing_length>,
                   LiteralStringPack<X...>, LiteralStringPack<false, A>,
                   LiteralStringPack<false, Y>...)
    -> decltype(KeepBeforeEnd(RemainderInfo<false, (trailing_length - 1)>(),
                              LiteralStringPack<false, X..., A>(),
                              LiteralStringPack<false, Y>()...));

// ExpandLongLiteral is the entry point for producing a template type whose
// parameter pack contains only the characters of the string literal (not
// including the trailing NUL that the compiler adds to terminate the string
// literal).
template <bool length_ok, size_t trailing_length, char... C>
auto ExpandLongLiteral(TotalLengthInfo<length_ok, trailing_length>,
                       LiteralStringPack<C...>)
    -> decltype(KeepBeforeEnd(
        RemainderInfo<(trailing_length >= kCharGroupSize), trailing_length>(),
        LiteralStringPack<C>()...));

// Specialization for empty strings. In theory we shouldn't need this, but in
// practice it avoids problems.
template <bool length_ok, char... C>
auto ExpandLongLiteral(TotalLengthInfo<length_ok, 0>, LiteralStringPack<C...>)
    -> LiteralStringPack<>;

// Specialization for strings that are are too long.
template <size_t trailing_length, char... C>
auto ExpandLongLiteral(TotalLengthInfo<false, trailing_length>,
                       LiteralStringPack<C...>) -> StringLiteralIsTooLong;

// Max length 31 (not including trailing NUL).

#define _TAS_IL2_KEEP_LITERAL_BEFORE_NUL_32(x)                          \
  decltype(NAMESPACE ExpandLongLiteral(                                 \
      NAMESPACE TotalLengthInfo<(sizeof x) - 1 < 32, (sizeof x) - 1>(), \
      NAMESPACE LiteralStringPack<_TAS_EXPAND_32(, x)>()))

#define _TAS_IL2_PSD_TYPE_32(x) \
  decltype(NAMESPACE ProvideStorage(_TAS_IL2_KEEP_LITERAL_BEFORE_NUL_32(x)()))

#define TAS_IL2_PSV_32(x) \
  (NAMESPACE MakeProgmemStringView<_TAS_IL2_PSD_TYPE_32(x)>())

#define TAS_IL2_FLASHSTR_32(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_IL2_PSD_TYPE_32(x)::kData))

// Max length 1023 (not including trailing NUL).

#define _TAS_IL2_KEEP_LITERAL_BEFORE_NUL_1024(x)                          \
  decltype(NAMESPACE ExpandLongLiteral(                                   \
      NAMESPACE TotalLengthInfo<(sizeof x) - 1 < 1024, (sizeof x) - 1>(), \
      NAMESPACE LiteralStringPack<_TAS_EXPAND_1024(, x)>()))

#define _TAS_IL2_PSD_TYPE_1024(x) \
  decltype(NAMESPACE ProvideStorage(_TAS_IL2_KEEP_LITERAL_BEFORE_NUL_1024(x)()))

#define TAS_IL2_PSV_1024(x) \
  (NAMESPACE MakeProgmemStringView<_TAS_IL2_PSD_TYPE_1024(x)>())

#define TAS_IL2_FLASHSTR_1024(x)                 \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _TAS_IL2_PSD_TYPE_1024(x)::kData))

TEST(FooTest, Bar) {
  using tli = TotalLengthInfo<true, 28>;
  using lsp = LiteralStringPack<'E', 'c', 'h', 'o', ',', ' ', 'e', 'c', 'h',
                                'o', ',', ' ', 'e', 'c', 'h', 'o', ',', ' ',
                                'e', 'c', 'h', 'o', ',', ' ', 'e', 'c', 'h',
                                'o', '\x00', '\x00', '\x00', '\x00'>;

  using abc = decltype(KeepBeforeEnd(RemainderInfo<(2 >= kCharGroupSize), 2>(),
                                     LiteralStringPack<'E'>(),
                                     LiteralStringPack<'c'>()));

  //  using xxx = decltype(ExpandLongLiteral(tli(), lsp()));
}

// TEST(FooTest, Bar) {
//   // EXPECT_EQ(10000000, sizeof "abcde");
//   // EXPECT_EQ(10000000, sizeof "abcd\0\0\0\0\0\0\0\0\0\0\0\0e");

//   auto flashstr = TAS_IL2_FLASHSTR_32("Echo, echo, echo, echo, echo");

//   mcucore::test::PrintToStdString out;
//   EXPECT_EQ(out.print(flashstr), 28);
//   EXPECT_EQ(out.str(), "Echo, echo, echo, echo, echo");
// }

TEST(InlineLiteralTest, ExplicitProgmemStrData) {
  using Type = ProgmemStrData<'H', 'E', 'L', 'L', 'O'>;
  auto printable = mcucore::progmem_data::MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 5);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 5);
  EXPECT_EQ(out.str(), "HELLO");
}

TEST(InlineLiteralTest, EmptyProgmemStrData) {
  using Type = ProgmemStrData<>;
  auto printable = mcucore::progmem_data::MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 0);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(InlineLiteralTest, TasExpand16) {
  // When we don't search for the NUL at the end, the ProgmemStrData is padded
  // out to the size nnn, determined by the _TAS_EXPAND_nnn used.
  using Type = ProgmemStrData<_TAS_EXPAND_16(, "Hello!")>;
  auto printable = mcucore::progmem_data::MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 16);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 16);
  EXPECT_EQ(out.str(), std::string_view("Hello!\0\0\0\0\0\0\0\0\0\0", 16));
}

TEST(InlineLiteralTest, PrintTasFlashstr) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(TAS_FLASHSTR("Echo, echo, echo, echo, echo")), 28);
  EXPECT_EQ(out.str(), "Echo, echo, echo, echo, echo");
}

// TEST(InlineLiteralTest, PrintEmptyTasFlashstr) {
//   mcucore::test::PrintToStdString out;
//   EXPECT_EQ(out.print(TAS_FLASHSTR("")), 0);
//   EXPECT_EQ(out.str(), "");
// }

// TEST(InlineLiteralTest, StreamTasFlashstr) {
//   mcucore::test::PrintToStdString out;
//   OPrintStream strm(out);
//   strm << TAS_FLASHSTR("foo, Bar, BAZ");
//   EXPECT_EQ(out.str(), "foo, Bar, BAZ");
// }

// TEST(InlineLiteralTest, LeadingNUL) {
//   using Type = _TAS_PSD_TYPE_128("\0abc");
//   EXPECT_EQ(1, sizeof(Type::kData));
//   mcucore::test::PrintToStdString out;
//   EXPECT_EQ(out.print(TAS_FLASHSTR_128("\0abc")), 0);
//   EXPECT_EQ(out.str(), "");
// }

// TEST(InlineLiteralTest, TasPsvPrintTo) {
//   mcucore::test::PrintToStdString out;
//   EXPECT_EQ(TAS_PSV("Hey There").printTo(out), 9);
//   EXPECT_EQ(out.str(), "Hey There");
//   EXPECT_EQ(TAS_PSV("Hey There").size(), 9);
// }

// TEST(InlineLiteralTest, StreamTasPsv) {
//   mcucore::test::PrintToStdString out;
//   OPrintStream strm(out);
//   strm << TAS_PSV("Hey There");
//   EXPECT_EQ(out.str(), "Hey There");
// }

// TEST(InlineLiteralTest, TasPsvToProgmemStringView) {
//   ProgmemStringView progmem_string_view = TAS_PSV("Hey There");
//   EXPECT_EQ(progmem_string_view.size(), 9);
//   mcucore::test::PrintToStdString out;
//   EXPECT_EQ(progmem_string_view.printTo(out), 9);
//   EXPECT_EQ(out.str(), "Hey There");
// }

// TEST(InlineLiteralTest, TasPsvToLiteral) {
//   Literal literal = TAS_PSV("Hey There!");
//   EXPECT_EQ(literal.size(), 10);
//   mcucore::test::PrintToStdString out;
//   EXPECT_EQ(literal.printTo(out), 10);
//   EXPECT_EQ(out.str(), "Hey There!");
// }

// TEST(InlineLiteralTest, StreamTasLit) {
//   mcucore::test::PrintToStdString out;
//   OPrintStream strm(out);
//   strm << TASLIT("Echo, echo, etc");
//   EXPECT_EQ(out.str(), "Echo, echo, etc");
// }

// ////////////////////////////////////////////////////////////////////////////////
// // Tests of the upper limits of string literal length.

// #define STR31 "abcdefghijklmnopqrstuvwxyz01234"
// #define STR32 STR31 "5"
// #define STR33 STR32 "6"

// #define STR63 STR32 STR31
// #define STR64 STR32 STR32
// #define STR65 STR32 STR33

// #define STR127 STR64 STR63
// #define STR128 STR64 STR64
// #define STR129 STR64 STR65

// #define STR255 STR128 STR127
// #define STR256 STR128 STR128
// #define STR257 STR128 STR129

// #define STR511 STR256 STR255
// #define STR512 STR256 STR256
// #define STR513 STR256 STR257

// #define STR1023 STR512 STR511
// #define STR1024 STR512 STR512
// #define STR1025 STR512 STR513

// #define FITS_TEST(x, len, max)                        \
//   TEST(InlineLiteralTest, Literal##len##Fits##max) {  \
//     using Type = _TAS_PSD_TYPE_##max(x);              \
//     EXPECT_EQ(len + 1, sizeof(Type::kData));          \
//     mcucore::test::PrintToStdString out;              \
//     EXPECT_EQ(out.print(TAS_FLASHSTR_##max(x)), len); \
//     EXPECT_EQ(out.str(), x);                          \
//   }

// FITS_TEST(STR31, 31, 32);
// FITS_TEST(STR63, 63, 64);
// FITS_TEST(STR127, 127, 128);
// FITS_TEST(STR255, 255, 256);
// FITS_TEST(STR511, 511, 512);

// // Each of the following static_asserts should fail to compile, hence each is
// // in an #ifdef block that, by default, is not active.

// #define OVERFLOWS_TEST(len, max)                                      \
//   using Type_##len##_Overflows_##max = _TAS_PSD_TYPE_##max(STR##len); \
//   static_assert(sizeof(Type_##len##_Overflows_##max) == 999999,       \
//                 "Should not compile.")

// #ifdef STR_32_OVERFLOWS_32
// OVERFLOWS_TEST(32, 32);
// #endif

// #ifdef STR_33_OVERFLOWS_32
// OVERFLOWS_TEST(33, 32);
// #endif

// #ifdef STR_64_OVERFLOWS_64
// OVERFLOWS_TEST(64, 64);
// #endif

// #ifdef STR_65_OVERFLOWS_64
// OVERFLOWS_TEST(65, 64);
// #endif

// #ifdef STR_128_OVERFLOWS_128
// OVERFLOWS_TEST(128, 128);
// #endif

// #ifdef STR_129_OVERFLOWS_128
// OVERFLOWS_TEST(129, 128);
// #endif

// #ifdef STR_256_OVERFLOWS_256
// OVERFLOWS_TEST(256, 256);
// #endif

// #ifdef STR_257_OVERFLOWS_256
// OVERFLOWS_TEST(257, 256);
// #endif

// #ifdef STR_512_OVERFLOWS_512
// OVERFLOWS_TEST(512, 512);
// #endif

// #ifdef STR_513_OVERFLOWS_512
// OVERFLOWS_TEST(513, 512);
// #endif

// #ifdef STR_1024_OVERFLOWS_1024
// // This will probably just exceed the compiler's ability to expand it.
// OVERFLOWS_TEST(1024, 1024);
// #endif

// #ifdef STR_1025_OVERFLOWS_1024
// // This will probably just exceed the compiler's ability to expand it.
// OVERFLOWS_TEST(1025, 1024);
// #endif
}  // namespace
}  // namespace test
}  // namespace mcucore
