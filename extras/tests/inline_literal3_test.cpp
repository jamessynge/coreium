// Goal: address the exceedingly slow performance of compiling TAS_PSV_1024.
// The macro results in a very long chain of template "expansions". sizeof S,
// where S is a string literal, evaluates to the size of that literal, including
// the trailing null character. We may be able to use this to reduce the number
// of steps in the chain, in exchange for having more unique templates
// specializations.
//
// Note that using this approach will skip checking for NUL before the end, and
// thus doesn't help us with detecting string literals that have embedded NULs,
// and such string literals don't work well in situations where we need to treat
// them as C-style NUL terminated strings (i.e. the null character appears
// before the end of the string.

#include <cstddef>
#include <ostream>
#include <regex>  // NOLINT
#include <string>
#include <string_view>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/test_strings.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"
#include "o_print_stream.h"
#include "progmem_string_view.h"

namespace mcucore {
namespace progmem_data {

// Get the Nth char from a string literal of length M, where that length
// includes the trailing null character at index M-1. This is used to produce
// the comma separated lists of chars that make up a literal string. If N is >
// M, the trailing null character is returned; as a result, _TAS_EXPAND_16(,
// "Hello") becomes:
//
//   'H','e','l','l','o','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'
template <int N, int M>
constexpr char GetNthCharOfM(char const (&c)[M]) {
  return c[N < M ? N : M - 1];
}

// Instantiations of this template provide static, constexpr storage for string
// literals. By placing them in the ::mcucore::progmem_data namespace, the
// linker will combine multiple occurrences of the same
// TAS_FLASHSTR(string_literal) across multiple files such that they share the
// storage.
template <char... C>
struct ProgmemStrData final {
  // We add a trailing null character here so that we can interpret kData as a
  // __FlashStringHelper instance (see TAS_FLASHSTR for how we do that);
  // Arduino's Print::print(const __FlashStringHelper*) needs the string to be
  // NUL terminated so that it knows when it has found the end of the string.
  static constexpr char const kData[sizeof...(C) + 1] AVR_PROGMEM = {C..., 0};
};

template <char... C>
constexpr char const ProgmemStrData<C...>::kData[sizeof...(C) + 1] AVR_PROGMEM;

template <class PSS>
ProgmemStringView MakeProgmemStringView() {
  return ProgmemStringView(PSS::kData, (sizeof PSS::kData) - 1);
}

template <bool BeforeNull, bool AfterLastChar, char... C>
struct Phase1StringFragment final {};

template <bool FoundNUL, char... C>
struct Phase2StringFragment final {};

template <char... C>
struct FullStringFragment final {};

class EmptyString;

class StringLiteralIsTooLong;

// When KeepBeforeNull has a single parameter, we've reached the end of of our
// search of the parameter pack for NUL. We may or may not have found the null
// character.
template <bool FoundNUL, char... C>
auto KeepBeforeNull(Phase2StringFragment<FoundNUL, C...>)  // as is...
    -> Phase2StringFragment<FoundNUL, C...>;

// template <char A, char... X, char... Y>
// auto KeepBeforeNull(Phase2StringFragment<false, A, X...>,
//                     Phase2StringFragment<false, '\0'>,
//                     Phase2StringFragment<false, Y>...)
//     -> Phase2StringFragment<true, A, X...>;

template <char... X, char... Y>
auto KeepBeforeNull(Phase2StringFragment<false, X...>,
                    Phase2StringFragment<false, '\0'>,
                    Phase2StringFragment<false, Y>...)
    -> Phase2StringFragment<true, X...>;

// For any character A other than '\0', append it to the template parameter pack
// X, then consider the characters following A.
template <char A, char... X, char... Y>
auto KeepBeforeNull(Phase2StringFragment<false, X...>,
                    Phase2StringFragment<false, A>,
                    Phase2StringFragment<false, Y>...)
    -> decltype(KeepBeforeNull(Phase2StringFragment<false, X..., A>(),
                               Phase2StringFragment<false, Y>()...));

// Matches a string fragment (C...) entirely in the string, i.e. before the
// terminating null character. The return type indicates that the null character
// has not yet been found (i.e. is after this fragment).
template <char... C>
auto DiscardAfterNull(Phase1StringFragment<true, false, C...>)
    -> Phase2StringFragment<false, C...>;

// Matches a string fragment (C...) entirely after the end of the string, which
// may start with the terminating null character. The return type indicates that
// the null character is at or before this fragment.
template <char... C>
auto DiscardAfterNull(Phase1StringFragment<false, true, C...>)
    -> Phase2StringFragment<true>;

// Matches a string fragment which includes the end of the string.
template <char... C>
auto DiscardAfterNull(Phase1StringFragment<false, false, C...>)
    -> decltype(KeepBeforeNull(Phase2StringFragment<false, C>()...));

// Concatenate string fragments...

// First fragment is entirely inside the string, the second fragment may be
// entirely before the end of the string, may record the end of the string, or
// may be after the end of the string.
template <bool FoundNUL, char... X, char... Y>
auto Concat(Phase2StringFragment<false, X...>,
            Phase2StringFragment<FoundNUL, Y...>)
    -> Phase2StringFragment<FoundNUL, X..., Y...>;

// First fragment either has the final characters of the string (not including
// the null termination), or is entirely after the end of the string; the second
// fragment is definitely after the last character.
template <char... X>
auto Concat(Phase2StringFragment<true, X...>, Phase2StringFragment<true>)
    -> Phase2StringFragment<true, X...>;

// If we were able to find the NUL at the end of the string literal, then
// ProvideStorage will return a type that has a static array with the string in
// it.
template <char... C>
auto ProvideStorage(Phase2StringFragment<true, C...>) -> ProgmemStrData<C...>;

// Else if the literal is too long for the expension macro used, ProvideStorage
// will return a type that isn't useful for our purposes below, and whose name
// hints at the problem.
template <char... C>
auto ProvideStorage(Phase2StringFragment<false, C...>)
    -> StringLiteralIsTooLong;

}  // namespace progmem_data

////////////////////////////////////////////////////////////////////////////////
// Macros for expanding a string literal into a comma separated list of
// characters of a hard coded length. If that length is longer than the string
// literal's length, the trailing characters are all NULs.

#define _TAS_GET_NTH_CHAR(n, x) ::mcucore::progmem_data::GetNthCharOfM<0x##n>(x)

#define _TAS_IL3_CONCAT_TYPE(t1, t2) \
  decltype(::mcucore::progmem_data::Concat(t1(), t2()))

// Make a Phase1StringFragment for a portion of the string |x| starting at
// offset 0x##n##0, where n is zero or more hexadecimal digits; there is no
// leading 0x before the digits.
//
// TODO(jamessynge) Consider whether to allow a string with N characters (not
// including the terminating null character) to be handled by a macro which
// supports strings of at most N characters. We have the advantage here that we
// know the offset of the null character (i.e. (sizeof x) - 1), and thus can
// determine that we don't need to examine the characters after the fragment
// that contains the character immediately before the final (null) character.
#define _TAS_IL3_PHASE1_TYPE(n, x)                                   \
  ::mcucore::progmem_data::Phase1StringFragment<                     \
      (0x##n##F < ((sizeof x) - 1)), (0x##n##0 >= ((sizeof x) - 1)), \
      _TAS_GET_NTH_CHAR(n##0, x), _TAS_GET_NTH_CHAR(n##1, x),        \
      _TAS_GET_NTH_CHAR(n##2, x), _TAS_GET_NTH_CHAR(n##3, x),        \
      _TAS_GET_NTH_CHAR(n##4, x), _TAS_GET_NTH_CHAR(n##5, x),        \
      _TAS_GET_NTH_CHAR(n##6, x), _TAS_GET_NTH_CHAR(n##7, x),        \
      _TAS_GET_NTH_CHAR(n##8, x), _TAS_GET_NTH_CHAR(n##9, x),        \
      _TAS_GET_NTH_CHAR(n##A, x), _TAS_GET_NTH_CHAR(n##B, x),        \
      _TAS_GET_NTH_CHAR(n##C, x), _TAS_GET_NTH_CHAR(n##D, x),        \
      _TAS_GET_NTH_CHAR(n##E, x), _TAS_GET_NTH_CHAR(n##F, x)>

/* 2^4 = 16 */
#define _TAS_IL3_FRAGMENT_TYPE(n, x)                  \
  decltype(::mcucore::progmem_data::DiscardAfterNull( \
      _TAS_IL3_PHASE1_TYPE(n, x)()))

#define _TAS_IL3_CONCAT_TOKENS(token1, token2) token1##token2

#define _TAS_IL3_CONCAT_FRAGMENTS_TYPE(hex0, hex1, hex2, x)          \
  _TAS_IL3_CONCAT_TYPE(                                              \
      _TAS_IL3_FRAGMENT_TYPE(_TAS_IL3_CONCAT_TOKENS(hex0, hex1), x), \
      _TAS_IL3_FRAGMENT_TYPE(_TAS_IL3_CONCAT_TOKENS(hex0, hex2), x))

/* 2^5 = 32 */
#define _TAS_IL3_CONCAT_32_TYPE(n, x) _TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 0, 1, x)

/* 2^6 = 64 */
#define _TAS_IL3_CONCAT_64_TYPE(n, x)                              \
  _TAS_IL3_CONCAT_TYPE(_TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 0, 1, x), \
                       _TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 2, 3, x))

/* 2^7 = 128 */
#define _TAS_IL3_CONCAT_128_TYPE(n, x)                                  \
  _TAS_IL3_CONCAT_TYPE(                                                 \
      _TAS_IL3_CONCAT_TYPE(_TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 0, 1, x),  \
                           _TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 2, 3, x)), \
      _TAS_IL3_CONCAT_TYPE(_TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 4, 5, x),  \
                           _TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 6, 7, x)))

/* 2^8 = 256 */
#define _TAS_IL3_CONCAT_256_TYPE(n, x)                                       \
  _TAS_IL3_CONCAT_TYPE(                                                      \
      _TAS_IL3_CONCAT_TYPE(                                                  \
          _TAS_IL3_CONCAT_TYPE(_TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 0, 1, x),   \
                               _TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 2, 3, x)),  \
          _TAS_IL3_CONCAT_TYPE(_TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 4, 5, x),   \
                               _TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 6, 7, x))), \
      _TAS_IL3_CONCAT_TYPE(                                                  \
          _TAS_IL3_CONCAT_TYPE(_TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, 8, 9, x),   \
                               _TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, A, B, x)),  \
          _TAS_IL3_CONCAT_TYPE(_TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, C, D, x),   \
                               _TAS_IL3_CONCAT_FRAGMENTS_TYPE(n, E, F, x))))

/* 2^9 = 512 */
#define _TAS_IL3_CONCAT_512_TYPE(n, x)                           \
  _TAS_IL3_CONCAT_TYPE(                                          \
      _TAS_IL3_CONCAT_256_TYPE(_TAS_IL3_CONCAT_TOKENS(n, 0), x), \
      _TAS_IL3_CONCAT_256_TYPE(_TAS_IL3_CONCAT_TOKENS(n, 1), x))

/* 2^10 = 1024 */
// WARNING: 1024 is too long in practice, clang gives up on a very beefy
// workstation, avr-gcc probably crashes my laptop.
#define _TAS_IL3_CONCAT_1024_TYPE(n, x)                               \
  _TAS_IL3_CONCAT_TYPE(                                               \
      _TAS_IL3_CONCAT_TYPE(                                           \
          _TAS_IL3_CONCAT_256_TYPE(_TAS_IL3_CONCAT_TOKENS(n, 0), x),  \
          _TAS_IL3_CONCAT_256_TYPE(_TAS_IL3_CONCAT_TOKENS(n, 1), x)), \
      _TAS_IL3_CONCAT_TYPE(                                           \
          _TAS_IL3_CONCAT_256_TYPE(_TAS_IL3_CONCAT_TOKENS(n, 2), x),  \
          _TAS_IL3_CONCAT_256_TYPE(_TAS_IL3_CONCAT_TOKENS(n, 3), x)))

////////////////////////////////////////////////////////////////////////////////
// Macros for using type deduction to produce a unique template instantiation
// for a string literal, with storage for just the characters of the string
// literal, but not more.
//
// _TAS_KEEP_LITERAL_BEFORE_NUL_nnn expands a string literal whose length is
// less than nnn to a LiteralStringPack instantiation for that string literal.
// If the string is too long, the expansion is instead to the type
// StringLiteralIsTooLong, whose name is intended to clue the developer in about
// the problem.
//
// _TAS_PSD_TYPE_nnn expands a string literal whose length is less than nnn to a
// ProgmemStrData instantiation for that string literal.

// Max length 31 (not including trailing NUL).

#define _TAS_IL3_PSD_TYPE_32(x)                     \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_IL3_CONCAT_32_TYPE(, x)()))

#define TAS_IL3_PSV_32(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_IL3_PSD_TYPE_32(x)>())

#define TAS_IL3_FLASHSTR_32(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_IL3_PSD_TYPE_32(x)::kData))

// Max length 63 (not including trailing NUL).

#define _TAS_IL3_PSD_TYPE_64(x)                     \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_IL3_CONCAT_64_TYPE(, x)()))

#define TAS_IL3_PSV_64(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_IL3_PSD_TYPE_64(x)>())

#define TAS_IL3_FLASHSTR_64(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_IL3_PSD_TYPE_64(x)::kData))

// Max length 127 (not including trailing NUL).

#define _TAS_IL3_PSD_TYPE_128(x)                    \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_IL3_CONCAT_128_TYPE(, x)()))

#define TAS_IL3_PSV_128(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_IL3_PSD_TYPE_128(x)>())

#define TAS_IL3_FLASHSTR_128(x)                  \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _TAS_IL3_PSD_TYPE_128(x)::kData))

// Max length 255 (not including trailing NUL).

#define _TAS_IL3_PSD_TYPE_256(x)                    \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_IL3_CONCAT_256_TYPE(, x)()))

#define TAS_IL3_PSV_256(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_IL3_PSD_TYPE_256(x)>())

#define TAS_IL3_FLASHSTR_256(x)                  \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _TAS_IL3_PSD_TYPE_256(x)::kData))

// Max length 511 (not including trailing NUL).

#define _TAS_IL3_PSD_TYPE_512(x)                    \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_IL3_CONCAT_512_TYPE(, x)()))

#define TAS_IL3_PSV_512(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_IL3_PSD_TYPE_512(x)>())

#define TAS_IL3_FLASHSTR_512(x)                  \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _TAS_IL3_PSD_TYPE_512(x)::kData))

// Max length 1023 (not including trailing NUL).

#define _TAS_IL3_PSD_TYPE_1024(x)                   \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_IL3_CONCAT_1024_TYPE(, x)()))

#define TAS_IL3_PSV_1024(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_IL3_PSD_TYPE_1024(x)>())

#define TAS_IL3_FLASHSTR_1024(x)                 \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _TAS_IL3_PSD_TYPE_1024(x)::kData))

////////////////////////////////////////////////////////////////////////////////
// Because we expect almost all string literals to be relatively short, we
// define these macros without specific lengths in their names for use in most
// of the code base. Where a longer string literal is required, use the
// appropriate macro defined above whose name specifies the next larger size
// limit.

#define TAS_PSV(x) TAS_IL3_PSV_64(x)
#define TAS_FLASHSTR(x) TAS_IL3_FLASHSTR_64(x)
#define TASLIT(x) TAS_IL3_PSV_128(x)

namespace progmem_data {
namespace test {
namespace {

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

TEST(Phase1TypeTest, FirstFragment) {
  using Type0 = _TAS_IL3_PHASE1_TYPE(0, "");
  std::cout << "Type0: " << PrettyTypeName<Type0>() << std::endl;

  using Type1 = _TAS_IL3_PHASE1_TYPE(0, "a");
  std::cout << "Type1: " << PrettyTypeName<Type1>() << std::endl;

  using Type2 = _TAS_IL3_PHASE1_TYPE(0, "ab");
  std::cout << "Type2: " << PrettyTypeName<Type2>() << std::endl;

  using Type14 = _TAS_IL3_PHASE1_TYPE(0, "abcdefghijklmn");
  std::cout << "Type14: " << PrettyTypeName<Type14>() << std::endl;

  using Type15 = _TAS_IL3_PHASE1_TYPE(0, "abcdefghijklmno");
  std::cout << "Type15: " << PrettyTypeName<Type15>() << std::endl;

  using Type16 = _TAS_IL3_PHASE1_TYPE(0, "abcdefghijklmnop");
  std::cout << "Type16: " << PrettyTypeName<Type16>() << std::endl;

  using Type17 = _TAS_IL3_PHASE1_TYPE(0, "abcdefghijklmnopq");
  std::cout << "Type17: " << PrettyTypeName<Type17>() << std::endl;
}

TEST(Phase1TypeTest, SecondFragment) {
  using Type0 = _TAS_IL3_PHASE1_TYPE(1, "");
  std::cout << "Type0: " << PrettyTypeName<Type0>() << std::endl;

  using Type14 = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmn");
  std::cout << "Type14: " << PrettyTypeName<Type14>() << std::endl;

  using Type15 = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmno");
  std::cout << "Type15: " << PrettyTypeName<Type15>() << std::endl;

  using Type16 = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnop");
  std::cout << "Type16: " << PrettyTypeName<Type16>() << std::endl;

  using Type17 = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopq");
  std::cout << "Type17: " << PrettyTypeName<Type17>() << std::endl;

  using Type18 = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopqr");
  std::cout << "Type18: " << PrettyTypeName<Type18>() << std::endl;

  using Type31 = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopqrstuvwxyz01234");
  std::cout << "Type31: " << PrettyTypeName<Type31>() << std::endl;

  using Type32 = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopqrstuvwxyz012345");
  std::cout << "Type32: " << PrettyTypeName<Type32>() << std::endl;

  using Type33 = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopqrstuvwxyz0123456");
  std::cout << "Type33: " << PrettyTypeName<Type33>() << std::endl;
}

TEST(KeepBeforeNullTest, Empty) {
  {
    using T = Phase2StringFragment<false>;
    using K = decltype(KeepBeforeNull(T()));
    std::cout << "<false>: " << PrettyTypeName<K>() << std::endl;
  }
  {
    using T = Phase2StringFragment<true>;
    using K = decltype(KeepBeforeNull(T()));
    std::cout << "<true>: " << PrettyTypeName<K>() << std::endl;
  }
  {
    using T = Phase2StringFragment<false, '\x00'>;
    using K = decltype(KeepBeforeNull(T()));
    std::cout << "<false, NUL>: " << PrettyTypeName<K>() << std::endl;
  }
  {
    using T = Phase2StringFragment<true, '\x00'>;
    using K = decltype(KeepBeforeNull(T()));
    std::cout << "<true, NUL>: " << PrettyTypeName<K>() << std::endl;
  }
  {
    using T = Phase2StringFragment<false, '\x00', '\x00'>;
    using K = decltype(KeepBeforeNull(T()));
    std::cout << "<false, NUL, NUL>: " << PrettyTypeName<K>() << std::endl;
  }
  {
    using T = Phase2StringFragment<false, '\x00'>;
    using K = decltype(KeepBeforeNull(T(), T()));
    std::cout << "<false, NUL>, <false, NUL>: " << PrettyTypeName<K>()
              << std::endl;
  }
}

TEST(KeepBeforeNullTest, LengthN) {
#undef P2CHAR
#define P2CHAR(c) Phase2StringFragment<false, c>()

  using K1 = decltype(KeepBeforeNull(P2CHAR('a')));
  using K2 = decltype(KeepBeforeNull(P2CHAR('a'), P2CHAR('b')));
  using K3 = decltype(KeepBeforeNull(P2CHAR('a'), P2CHAR('b'), P2CHAR('c')));

  std::cout << "K1: " << PrettyTypeName<K1>() << std::endl;
  std::cout << "K2: " << PrettyTypeName<K2>() << std::endl;
  std::cout << "K3: " << PrettyTypeName<K3>() << std::endl;

  using K1z = decltype(KeepBeforeNull(P2CHAR('a'), P2CHAR('\x00')));
  using K2z = decltype(KeepBeforeNull(P2CHAR('a'), P2CHAR('b'), P2CHAR('\x00'),
                                      P2CHAR('\x00'), P2CHAR('\x00')));
  using K3z =
      decltype(KeepBeforeNull(P2CHAR('a'), P2CHAR('b'), P2CHAR('c'),
                              P2CHAR('\x00'), P2CHAR('\x00'), P2CHAR('\x00')));

  std::cout << "K1z: " << PrettyTypeName<K1z>() << std::endl;
  std::cout << "K2z: " << PrettyTypeName<K2z>() << std::endl;
  std::cout << "K3z: " << PrettyTypeName<K3z>() << std::endl;

#undef P2CHAR
}

TEST(DiscardAfterNullTest, EmptyString) {
  {
    using T = _TAS_IL3_PHASE1_TYPE(, "");
    std::cout << "T = (, \"\"): " << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(0, "");
    std::cout << "T = (0, \"\"): " << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(1, "");
    std::cout << "T = (1, \"\"): " << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(2, "");
    std::cout << "T = (2, \"\"): " << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
}

TEST(DiscardAfterNullTest, FirstFragment) {
  {
    using T = _TAS_IL3_PHASE1_TYPE(0, "a");
    std::cout << "T = (0, \"a\"): " << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(0, "abcdefghijklmno");
    std::cout << "T = (0, \"p\"): " << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(0, "abcdefghijklmnop");
    std::cout << "T = (0, \"abcdefghijklmnop\"): " << PrettyTypeName<T>()
              << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(0, "abcdefghijklmnopq");
    std::cout << "T = (0, \"abcdefghijklmnopq\"): " << PrettyTypeName<T>()
              << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
}

TEST(DiscardAfterNullTest, SecondFragment) {
  {
    using T = _TAS_IL3_PHASE1_TYPE(1, "a");
    std::cout << "T = (1, \"a\"): " << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnop");
    std::cout << "T = (1, \"abcdefghijklmnop\"): " << PrettyTypeName<T>()
              << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopq");
    std::cout << "T = (1, \"abcdefghijklmnopq\"): " << PrettyTypeName<T>()
              << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopqrstuvwxyz01234");
    std::cout << "T = (1, \"abcdefghijklmnopqrstuvwxyz01234\"): "
              << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopqrstuvwxyz012345");
    std::cout << "T = (1, \"abcdefghijklmnopqrstuvwxyz012345\"): "
              << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
  {
    using T = _TAS_IL3_PHASE1_TYPE(1, "abcdefghijklmnopqrstuvwxyz0123456");
    std::cout << "T = (1, \"abcdefghijklmnopqrstuvwxyz0123456\"): "
              << PrettyTypeName<T>() << std::endl;
    using D = decltype(DiscardAfterNull(T()));
    std::cout << "DiscardAfterNull(T()): " << PrettyTypeName<D>() << std::endl
              << std::endl;
  }
}

TEST(ConcatTest, NonMacroCases) {
  {
    using T1 = Phase2StringFragment<false, 'a'>;
    std::cout << "T21: " << PrettyTypeName<T1>() << std::endl;

    using T2 = Phase2StringFragment<false, 'b'>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
  }
  {
    using T1 = Phase2StringFragment<false, 'a'>;
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = Phase2StringFragment<true, 'b'>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
  }
  {
    using T1 = Phase2StringFragment<true, 'a'>;
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = Phase2StringFragment<true>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
  }
  {
    using T1 = Phase2StringFragment<true>;
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = Phase2StringFragment<true>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
  }
  // Degenerate cases that shouldn't happen, but which we don't treat as
  // invalid.
  {
    using T1 = Phase2StringFragment<false>;
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = Phase2StringFragment<false, 'B'>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
  }
  {
    using T1 = Phase2StringFragment<false, 'A'>;
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = Phase2StringFragment<false>;
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = decltype(Concat(T1(), T2()));
    std::cout << "Concat: " << PrettyTypeName<C>() << std::endl << std::endl;
  }
}

TEST(ConcatTest, MacroCases) {
  {
#define S "a"
    std::cout << "S: " << S << std::endl;

    using T1 = _TAS_IL3_FRAGMENT_TYPE(0, S);
    std::cout << "T1: " << PrettyTypeName<T1>() << std::endl;

    using T2 = _TAS_IL3_FRAGMENT_TYPE(1, S);
    std::cout << "T2: " << PrettyTypeName<T2>() << std::endl;

    using C = _TAS_IL3_CONCAT_TYPE(T1, T2);
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl;
#undef S
  }
  {
#define S "a"
    std::cout << "S: " << S << std::endl;

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmno"
    std::cout << "S: " << S << std::endl;

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnop"
    std::cout << "S: " << S << std::endl;

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopq"
    std::cout << "S: " << S << std::endl;

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz01234"
    std::cout << "S: " << S << std::endl;

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz012345"
    std::cout << "S: " << S << std::endl;

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz0123456"
    std::cout << "S: " << S << std::endl;

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, 0, 1, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
  }
  {
#define S "abcdefghijklmnopqrstuvwxyz0123456"
    std::cout << "S: " << S << std::endl;

#define A 0
#define B 1

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, A, B, S);
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

    using T = _TAS_IL3_CONCAT_FRAGMENTS_TYPE(0, A, B, S);
    std::cout << "T: " << PrettyTypeName<T>() << std::endl << std::endl;
#undef S
#undef A
#undef B
  }
}

TEST(ConcatTest, Concat32MacroCases) {
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _TAS_IL3_CONCAT_32_TYPE(n, x);                             \
    std::cout << "C: " << PrettyTypeName<C>() << std::endl << std::endl; \
  }

  MAKE_BLOCK(, "");
  MAKE_BLOCK(0, "");

  MAKE_BLOCK(0, "a");
  MAKE_BLOCK(1, "a");

  MAKE_BLOCK(0, "abcdefghijklmno");
  MAKE_BLOCK(1, "abcdefghijklmno");

  MAKE_BLOCK(0, "abcdefghijklmnop");
  MAKE_BLOCK(1, "abcdefghijklmnopq");

  MAKE_BLOCK(0, "abcdefghijklmnopqrstuvwxyz01234");
  MAKE_BLOCK(1, "abcdefghijklmnopqrstuvwxyz01234");

  MAKE_BLOCK(0, "abcdefghijklmnopqrstuvwxyz012345");
  MAKE_BLOCK(1, "abcdefghijklmnopqrstuvwxyz012345");

  MAKE_BLOCK(0, "abcdefghijklmnopqrstuvwxyz0123456");
  MAKE_BLOCK(1, "abcdefghijklmnopqrstuvwxyz0123456");

#undef MAKE_BLOCK
}

TEST(ConcatTest, Concat64MacroCases) {
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _TAS_IL3_CONCAT_64_TYPE(n, x);                             \
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
#undef MAKE_BLOCK
}

TEST(ConcatTest, Concat128MacroCases) {
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _TAS_IL3_CONCAT_128_TYPE(n, x);                            \
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
#undef MAKE_BLOCK
}

TEST(ConcatTest, Concat256MacroCases) {
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _TAS_IL3_CONCAT_256_TYPE(n, x);                            \
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

#undef MAKE_BLOCK
}

////////////////////////////////////////////////////////////////////////////////
// Tests of the upper limits of string literal length.

TEST(ConcatTest, Concat512MacroCases) {
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _TAS_IL3_CONCAT_512_TYPE(n, x);                            \
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
#undef MAKE_BLOCK
}

TEST(ConcatTest, Concat1024MacroCases) {
#define MAKE_BLOCK(n, x)                                                 \
  {                                                                      \
    std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
              << std::endl;                                              \
    using C = _TAS_IL3_CONCAT_1024_TYPE(n, x);                           \
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
#undef MAKE_BLOCK
}

// #define COMPILES_TEST(x, len)                                            \
//   TEST(ConcatTest, LiteralOfLength##len##Compiles) {                     \
//     std::cout << "n=" #n ", len(x)=" << ((sizeof x) - 1) << ", x: " << x \
//               << std::endl;                                              \
//     using Type = _TAS_PSD_TYPE_##max(x);                                 \
//     EXPECT_EQ(len + 1, sizeof(Type::kData));                             \
//     mcucore::test::PrintToStdString out;                                 \
//     EXPECT_EQ(out.print(TAS_FLASHSTR_##max(x)), len);                    \
//     EXPECT_EQ(out.str(), x);                                             \
//   }

// COMPILES_TEST(TEST_STR_31, 31, 32);
// COMPILES_TEST(TEST_STR_63, 63, 64);
// COMPILES_TEST(TEST_STR_127, 127, 128);
// COMPILES_TEST(TEST_STR_255, 255, 256);
// COMPILES_TEST(TEST_STR_511, 511, 512);

TEST(InlineLiteralTest, ExplicitProgmemStrData) {
  using Type = ProgmemStrData<'H', 'E', 'L', 'L', 'O'>;
  auto printable = MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 5);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 5);
  EXPECT_EQ(out.str(), "HELLO");
}

TEST(InlineLiteralTest, EmptyProgmemStrData) {
  using Type = ProgmemStrData<>;
  auto printable = MakeProgmemStringView<Type>();
  EXPECT_EQ(printable.size(), 0);
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(printable.printTo(out), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(InlineLiteralTest, Phase1StringFragmentHasNulls) {
  // When we don't search for the null character at the end, the template
  // class's parameter pack is padded out to have 16 characters.
  using Type = _TAS_IL3_PHASE1_TYPE(, "Hello!");
  EXPECT_THAT(PrettyTypeName<Type>(),
              testing::HasSubstr("'H', 'e', 'l', 'l', 'o', '!', NUL, NUL, NUL, "
                                 "NUL, NUL, NUL, NUL, NUL, NUL, NUL"));
}

TEST(InlineLiteralTest, PrintTasFlashstr) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(TAS_FLASHSTR("Echo, echo, echo, echo, echo")), 28);
  EXPECT_EQ(out.str(), "Echo, echo, echo, echo, echo");
}

TEST(InlineLiteralTest, PrintEmptyTasFlashstr) {
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(TAS_FLASHSTR("")), 0);
  EXPECT_EQ(out.str(), "");
}

TEST(InlineLiteralTest, StreamTasFlashstr) {
  mcucore::test::PrintToStdString out;
  OPrintStream strm(out);
  strm << TAS_FLASHSTR("foo, Bar, BAZ");
  EXPECT_EQ(out.str(), "foo, Bar, BAZ");
}

TEST(InlineLiteralTest, LeadingNUL) {
  using Type = _TAS_IL3_PSD_TYPE_128("\0abc");
  EXPECT_EQ(1, sizeof(Type::kData));
  mcucore::test::PrintToStdString out;
  EXPECT_EQ(out.print(TAS_IL3_FLASHSTR_128("\0abc")), 0);
  EXPECT_EQ(out.str(), "");
}

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

// // Each of the following static_asserts should fail to compile, hence each
// is
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
}  // namespace progmem_data
}  // namespace mcucore
