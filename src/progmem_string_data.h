#ifndef MCUCORE_SRC_PROGMEM_STRING_DATA_H_
#define MCUCORE_SRC_PROGMEM_STRING_DATA_H_

// Provides macros for storing string literals in program memory (PROGMEM)
// rather than RAM, when compiled with avr-gcc for the AVR line of processors.
// The linker should be able to collapse multiple occurrences of the same string
// literal into a single array in PROGMEM; this isn't true when using the
// Arduino defined F(string_literal) macro, where every occurrence in a single
// file is stored separately.
//
// TASLIT(string_literal) expands to a ProgmemStringView instance with
// string_literal as the value it views.
//
// TAS_FLASHSTR(string_literal) expands to a const __FlashStringHelper* pointer
// value, just as F(string_literal) does, but without the wasted storage.
//
// This is inspired by https://github.com/irrequietus/typestring, by George
// Makrydakis <george@irrequietus.eu>. typestring.hh expands a string literal
// passed to a macro function into a list of char values which is used as a
// template parameter pack; template metaprogramming is used to iterate through
// the characters looking for the string terminating null character, though
// recursive type deduction is used to perform that iteration. This works fine
// for short strings, but with longer strings (e.g. above 128) the recursive
// type deduction becomes rather slow, and can exceed the the stack size
// supported by the compiler.
//
// To avoid the compilation issues mentioned above, this implementation is
// different in these key ways:
//
// 1) The string treated as a sequence of 16 character string fragments, so that
//    whole fragments can be operated upon rather than individual characters.
//
// 2) Rather than search for the terminating null character, the compiler
//    determined size of the string literal is used to find the end of the
//    string.
//
// 3) Only the fragment containing the terminating null is processed to trim off
//    the terminating null and the following padding, which is also made up of
//    null characters.
//
// 4) Recursion is used in a binary tree fashion, rather than as tail recursion.
//    This means that the fragments 1 and 2 are concatenated together into
//    fragment A, fragments 3 and 4 are concatenated together into fragment B,
//    and finally fragments A and B are concatenated together into fragment C.
//    This limits the depth of the compiler stack required to perform type
//    deduction.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "progmem_string_view.h"  // IWYU pragma: export
#include "type_traits.h"

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
  static constexpr char const kData[1 + sizeof...(C)] AVR_PROGMEM = {C..., 0};
};

template <char... C>
constexpr char const ProgmemStrData<C...>::kData[1 + sizeof...(C)] AVR_PROGMEM;

// TODO Generate a compile time error if sizeof PSS:kData is too large to be
// represented by ProgmemStringView. E.g. create a declared but unimplemented
// specialization of this function for that case whose return type is
// StringLiteralIsTooLong.
template <class PSS>
ProgmemStringView MakeProgmemStringView() {
  constexpr size_t size =
      (sizeof PSS::kData) - 1;  // Size without the null termination.
  static_assert(size <= ProgmemStringView::kMaxSize, "String is too long");
  return ProgmemStringView(PSS::kData, size);
}

// Phase1StringFragment is a fragment of a string literal AND of the trailing
// nulls used to pad it out to the fixed length supported by the macros below;
// this is required because the C++ Preprocessor does not support any control
// flow or conditionals within a macro expansion.
//
// * BeforeLastChar is true if the fragment contains (16) characters and does
//   not need to have trailing nulls discarded.
// * CharsToDiscard is meaningful only when BeforeLastChar and AfterLastChar are
//   both false, in which case it is the number of characters to remove from the
//   parameter pack C.
// * AfterLastChar is true if the fragment contains only trailing nulls and can
//   be discarded without any examination of the characters.
template <bool BeforeLastChar, int CharsToDiscard, bool AfterLastChar,
          char... C>
struct Phase1StringFragment final {};

// template <int CharsToDiscard, char... C>
// struct Phase2StringFragment final {};

template <char... C>
struct StringFragment final {};

// template <char... C>
// struct FullStringFragment final {};

template <int N>
struct DiscardCount final {};

template <bool LengthOK>
struct LengthCheck final {};

class EmptyString;

// "Forward" declaration of an undefined type. If this appears in a compiler
// error message for the expansion of TAS_FLASHSTR_nnn, or a related macro, it
// means that the string literal is too long (>= nnn).
class StringLiteralIsTooLong;

// KeepLiteral has specializations for each count of leading characters to be
// kept (1 through 15) each count of trailing nulls to be discarded (15 through
// 1).

// Keep 1 character, discard 15 nulls.
template <char C1, char... X>
auto KeepLiteral(DiscardCount<15>, StringFragment<C1>, StringFragment<X>...)
    -> StringFragment<C1>;

// Keep 2 characters, discard 14 nulls.
template <char C1, char C2, char... X>
auto KeepLiteral(DiscardCount<14>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<X>...) -> StringFragment<C1, C2>;

// Keep 3 characters, discard 13 nulls.
template <char C1, char C2, char C3, char... X>
auto KeepLiteral(DiscardCount<13>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3>;

// Keep 4 characters, discard 12 nulls.
template <char C1, char C2, char C3, char C4, char... X>
auto KeepLiteral(DiscardCount<12>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4>;

// Keep 5 characters, discard 11 nulls.
template <char C1, char C2, char C3, char C4, char C5, char... X>
auto KeepLiteral(DiscardCount<11>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<X>...) -> StringFragment<C1, C2, C3, C4, C5>;

// Keep 6 characters, discard 10 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char... X>
auto KeepLiteral(DiscardCount<10>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6>;

// Keep 7 characters, discard 9 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char... X>
auto KeepLiteral(DiscardCount<9>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7>;

// Keep 8 characters, discard 8 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char C8, char... X>
auto KeepLiteral(DiscardCount<8>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<C8>,
                 StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7, C8>;

// Keep 9 characters, discard 7 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char C8, char C9, char... X>
auto KeepLiteral(DiscardCount<7>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<C8>,
                 StringFragment<C9>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7, C8, C9>;

// Keep 10 characters, discard 6 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char C8, char C9, char C10, char... X>
auto KeepLiteral(DiscardCount<6>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<C8>,
                 StringFragment<C9>, StringFragment<C10>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7, C8, C9, C10>;

// Keep 11 characters, discard 5 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char C8, char C9, char C10, char C11, char... X>
auto KeepLiteral(DiscardCount<5>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<C8>,
                 StringFragment<C9>, StringFragment<C10>, StringFragment<C11>,
                 StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11>;

// Keep 12 characters, discard 4 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char C8, char C9, char C10, char C11, char C12, char... X>
auto KeepLiteral(DiscardCount<4>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<C8>,
                 StringFragment<C9>, StringFragment<C10>, StringFragment<C11>,
                 StringFragment<C12>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12>;

// Keep 13 characters, discard 3 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char C8, char C9, char C10, char C11, char C12, char C13, char... X>
auto KeepLiteral(DiscardCount<3>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<C8>,
                 StringFragment<C9>, StringFragment<C10>, StringFragment<C11>,
                 StringFragment<C12>, StringFragment<C13>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13>;

// Keep 14 characters, discard 2 nulls.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char C8, char C9, char C10, char C11, char C12, char C13, char C14,
          char... X>
auto KeepLiteral(DiscardCount<2>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<C8>,
                 StringFragment<C9>, StringFragment<C10>, StringFragment<C11>,
                 StringFragment<C12>, StringFragment<C13>, StringFragment<C14>,
                 StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13,
                      C14>;

// Keep 15 characters, discard 1 null.
template <char C1, char C2, char C3, char C4, char C5, char C6, char C7,
          char C8, char C9, char C10, char C11, char C12, char C13, char C14,
          char C15, char... X>
auto KeepLiteral(DiscardCount<1>, StringFragment<C1>, StringFragment<C2>,
                 StringFragment<C3>, StringFragment<C4>, StringFragment<C5>,
                 StringFragment<C6>, StringFragment<C7>, StringFragment<C8>,
                 StringFragment<C9>, StringFragment<C10>, StringFragment<C11>,
                 StringFragment<C12>, StringFragment<C13>, StringFragment<C14>,
                 StringFragment<C15>, StringFragment<X>...)
    -> StringFragment<C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13,
                      C14, C15>;

// Matches a string fragment (C...) entirely in the string, i.e. before the
// terminating null character.
template <int CharsToDiscard, char... C>
auto DiscardAfterLiteral(
    Phase1StringFragment<true, CharsToDiscard, false, C...>)
    -> StringFragment<C...>;

// Matches a string fragment (C...) entirely after the end of the string, which
// may start with the terminating null character.
template <int CharsToDiscard, char... C>
auto DiscardAfterLiteral(
    Phase1StringFragment<false, CharsToDiscard, true, C...>)
    -> StringFragment<>;

// Matches a string fragment which includes the end of the string and some
// trailing nulls.
template <int CharsToDiscard, char... C>
auto DiscardAfterLiteral(
    Phase1StringFragment<false, CharsToDiscard, false, C...>)
    -> decltype(KeepLiteral(DiscardCount<CharsToDiscard>(),
                            StringFragment<C>()...));

// Concatenate string fragments.
template <char... X, char... Y>
auto Concat(StringFragment<X...>, StringFragment<Y...>)
    -> StringFragment<X..., Y...>;

// If the macro used (e.g. TAS_PSV_nnn) supports strings as long as that
// provided, then ProvideStorage's return type will have a static array with
// the string in it.
template <char... C>
auto ProvideStorage(LengthCheck<true>, StringFragment<C...>)
    -> ProgmemStrData<C...>;

// Else if the literal is too long for the expension macro used, then
// ProvideStorage's return type will be StringLiteralIsTooLong, an undefined
// type whose name indicates what the problem is.
template <char... C>
auto ProvideStorage(LengthCheck<false>, StringFragment<C...>)
    -> StringLiteralIsTooLong;

}  // namespace progmem_data
}  // namespace mcucore

////////////////////////////////////////////////////////////////////////////////
// Macros using type deduction (i.e. using the above class and function
// templates) to produce a unique template instantiation for a string literal,
// with storage for just the characters of the string literal, but not more.
// Unique here means that if we use the same literal in multiple source files,
// the same type will be produced in all cases, and thus they'll all share the
// same storage after linking.
//
// We assume here that the expansion may occur in any namespace, thus we make
// absolute refererences to the types of the above templates.

// Expands to the n-th character of x; if n is greater than the size of x, then
// expands to the null character.
#define _TAS_GET_NTH_CHAR(n, x) ::mcucore::progmem_data::GetNthCharOfM<0x##n>(x)

// Value used to detect when the string literal is longer than is supported by
// the macro used to decompose the string literal into characters. The value is
// used to select the appropriate specialization of ProvideStorage.
#define _TAS_LENGTH_CHECK(x, max_chars) \
  (::mcucore::progmem_data::LengthCheck<((sizeof x) - 1 <= max_chars)>())

// Support for the working with individual string fragments of 16 characters.

// Expands to a Phase1StringFragment type for the portion of string |x| starting
// at offset 0x##n##0, where n is zero or more hexadecimal digits; there is no
// leading 0x before the digits in |n|. Any character offset that is beyond the
// last character of |x| is expanded to the null character.
#define _TAS_PHASE1_TYPE(n, x)                                                \
  ::mcucore::progmem_data::Phase1StringFragment<                              \
      /* BeforeLastChar */ ((sizeof x) >= 16 && 0x##n##F < ((sizeof x) - 1)), \
      /* CharsToDiscard */ (16 - (((sizeof x) - 1) % 16)),                    \
      /* AfterLastChar */ (0x##n##0 >= ((sizeof x) - 1)),                     \
      _TAS_GET_NTH_CHAR(n##0, x), _TAS_GET_NTH_CHAR(n##1, x),                 \
      _TAS_GET_NTH_CHAR(n##2, x), _TAS_GET_NTH_CHAR(n##3, x),                 \
      _TAS_GET_NTH_CHAR(n##4, x), _TAS_GET_NTH_CHAR(n##5, x),                 \
      _TAS_GET_NTH_CHAR(n##6, x), _TAS_GET_NTH_CHAR(n##7, x),                 \
      _TAS_GET_NTH_CHAR(n##8, x), _TAS_GET_NTH_CHAR(n##9, x),                 \
      _TAS_GET_NTH_CHAR(n##A, x), _TAS_GET_NTH_CHAR(n##B, x),                 \
      _TAS_GET_NTH_CHAR(n##C, x), _TAS_GET_NTH_CHAR(n##D, x),                 \
      _TAS_GET_NTH_CHAR(n##E, x), _TAS_GET_NTH_CHAR(n##F, x)>

// Expands to a string fragment without trailing nulls.
#define _TAS_FRAGMENT_TYPE(n, x)                         \
  decltype(::mcucore::progmem_data::DiscardAfterLiteral( \
      _TAS_PHASE1_TYPE(n, x)()))

// Expands to the type of two (adjacent) string fragments, t1() and t2(), being
// concatenated together.
#define _TAS_CONCAT_TYPE(t1, t2) \
  decltype(::mcucore::progmem_data::Concat(t1(), t2()))

// Concatenates two preprocessor tokens together. This is used below to ensure
// that two macro arguments are expanded before they are concatenated to produce
// a new token.
#define _TAS_CONCAT_TOKENS(token1, token2) token1##token2

// Expands to the type from concatenating the types of two fragments of 16
// characters starting at 0x##hex0##hex1##0 and at 0x##hex0##hex2##0.
#define _TAS_CONCAT_FRAGMENTS_TYPE(hex0, hex1, hex2, x)                   \
  _TAS_CONCAT_TYPE(_TAS_FRAGMENT_TYPE(_TAS_CONCAT_TOKENS(hex0, hex1), x), \
                   _TAS_FRAGMENT_TYPE(_TAS_CONCAT_TOKENS(hex0, hex2), x))

////////////////////////////////////////////////////////////////////////////////
// Macros for producing StringFragment types that support specific lengths of
// string literals.

/* 2^5 = 32 */
#define _TAS_CONCAT_32_TYPE(n, x) _TAS_CONCAT_FRAGMENTS_TYPE(n, 0, 1, x)

/* 2^6 = 64 */
#define _TAS_CONCAT_64_TYPE(n, x)                          \
  _TAS_CONCAT_TYPE(_TAS_CONCAT_FRAGMENTS_TYPE(n, 0, 1, x), \
                   _TAS_CONCAT_FRAGMENTS_TYPE(n, 2, 3, x))

/* 2^7 = 128 */
#define _TAS_CONCAT_128_TYPE(n, x)                                           \
  _TAS_CONCAT_TYPE(_TAS_CONCAT_TYPE(_TAS_CONCAT_FRAGMENTS_TYPE(n, 0, 1, x),  \
                                    _TAS_CONCAT_FRAGMENTS_TYPE(n, 2, 3, x)), \
                   _TAS_CONCAT_TYPE(_TAS_CONCAT_FRAGMENTS_TYPE(n, 4, 5, x),  \
                                    _TAS_CONCAT_FRAGMENTS_TYPE(n, 6, 7, x)))

/* 2^8 = 256 */
#define _TAS_CONCAT_256_TYPE(n, x)                                   \
  _TAS_CONCAT_TYPE(                                                  \
      _TAS_CONCAT_TYPE(                                              \
          _TAS_CONCAT_TYPE(_TAS_CONCAT_FRAGMENTS_TYPE(n, 0, 1, x),   \
                           _TAS_CONCAT_FRAGMENTS_TYPE(n, 2, 3, x)),  \
          _TAS_CONCAT_TYPE(_TAS_CONCAT_FRAGMENTS_TYPE(n, 4, 5, x),   \
                           _TAS_CONCAT_FRAGMENTS_TYPE(n, 6, 7, x))), \
      _TAS_CONCAT_TYPE(                                              \
          _TAS_CONCAT_TYPE(_TAS_CONCAT_FRAGMENTS_TYPE(n, 8, 9, x),   \
                           _TAS_CONCAT_FRAGMENTS_TYPE(n, A, B, x)),  \
          _TAS_CONCAT_TYPE(_TAS_CONCAT_FRAGMENTS_TYPE(n, C, D, x),   \
                           _TAS_CONCAT_FRAGMENTS_TYPE(n, E, F, x))))

/* 2^9 = 512 */
#define _TAS_CONCAT_512_TYPE(n, x)                                    \
  _TAS_CONCAT_TYPE(_TAS_CONCAT_256_TYPE(_TAS_CONCAT_TOKENS(n, 0), x), \
                   _TAS_CONCAT_256_TYPE(_TAS_CONCAT_TOKENS(n, 1), x))

/* 2^10 = 1024 */
#define _TAS_CONCAT_1024_TYPE(n, x)                                        \
  _TAS_CONCAT_TYPE(                                                        \
      _TAS_CONCAT_TYPE(_TAS_CONCAT_256_TYPE(_TAS_CONCAT_TOKENS(n, 0), x),  \
                       _TAS_CONCAT_256_TYPE(_TAS_CONCAT_TOKENS(n, 1), x)), \
      _TAS_CONCAT_TYPE(_TAS_CONCAT_256_TYPE(_TAS_CONCAT_TOKENS(n, 2), x),  \
                       _TAS_CONCAT_256_TYPE(_TAS_CONCAT_TOKENS(n, 3), x)))

////////////////////////////////////////////////////////////////////////////////
// We define below macros TAS_PSV_nnn (PSV==ProgmemStringView) and
// TAS_FLASHSTR_nnn for various values of nnn, which represents the maximum
// length of string literal (not including the terminating null character)
// supported by the macro. These produce *values* that can be printed or
// otherwise operated upon at runtime.
//
// These macros build on an internal use macro _TAS_PSD_TYPE_nnn
// (PSD==ProgmemStrData), whose expansion is a concrete template type.

// Max length 32:

#define _TAS_PSD_TYPE_32(x)                         \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_LENGTH_CHECK(x, 32), _TAS_CONCAT_32_TYPE(, x)()))

#define TAS_PSV_32(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_PSD_TYPE_32(x)>())

#define TAS_FLASHSTR_32(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_PSD_TYPE_32(x)::kData))

// Max length 64 (not including trailing NUL).

#define _TAS_PSD_TYPE_64(x)                         \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_LENGTH_CHECK(x, 64), _TAS_CONCAT_64_TYPE(, x)()))

#define TAS_PSV_64(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_PSD_TYPE_64(x)>())

#define TAS_FLASHSTR_64(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_PSD_TYPE_64(x)::kData))

// Max length 128 (not including trailing NUL).

#define _TAS_PSD_TYPE_128(x)                        \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_LENGTH_CHECK(x, 128), _TAS_CONCAT_128_TYPE(, x)()))

#define TAS_PSV_128(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_PSD_TYPE_128(x)>())

#define TAS_FLASHSTR_128(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_PSD_TYPE_128(x)::kData))

// Max length 255 (not including trailing NUL). This is not a power of two
// because ProgmemStringView uses a uint8 to record the size of the string, and
// can't represent 256.

#define _TAS_PSD_TYPE_255(x)                        \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_LENGTH_CHECK(x, 255), _TAS_CONCAT_256_TYPE(, x)()))

#define TAS_PSV_255(x) \
  (::mcucore::progmem_data::MakeProgmemStringView<_TAS_PSD_TYPE_255(x)>())

// Max length 256 (not including trailing NUL). There is no support here for
// ProgmemStringView because it can't support such a long string.

#define _TAS_PSD_TYPE_256(x)                        \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_LENGTH_CHECK(x, 256), _TAS_CONCAT_256_TYPE(, x)()))

#define TAS_FLASHSTR_256(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_PSD_TYPE_256(x)::kData))

// Max length 512 (not including trailing NUL). There is no support here for
// ProgmemStringView because it can't support such a long string.

#define _TAS_PSD_TYPE_512(x)                        \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_LENGTH_CHECK(x, 512), _TAS_CONCAT_512_TYPE(, x)()))

#define TAS_FLASHSTR_512(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_PSD_TYPE_512(x)::kData))

// Max length 1024 (not including trailing NUL). There is no support here for
// ProgmemStringView because it can't support such a long string.

#define _TAS_PSD_TYPE_1024(x)                       \
  decltype(::mcucore::progmem_data::ProvideStorage( \
      _TAS_LENGTH_CHECK(x, 1024), _TAS_CONCAT_1024_TYPE(, x)()))

#define TAS_FLASHSTR_1024(x) \
  (reinterpret_cast<const __FlashStringHelper*>(_TAS_PSD_TYPE_1024(x)::kData))

////////////////////////////////////////////////////////////////////////////////
// Because we expect almost all string literals to be relatively short, we
// define these macros without specific lengths in their names for use in most
// of the code base. Where a longer string literal is required, use the
// appropriate macro defined above whose name specifies the next larger size
// limit.

#define TAS_PSV(x) TAS_PSV_64(x)
#define TAS_FLASHSTR(x) TAS_FLASHSTR_64(x)
#define TASLIT(x) TAS_PSV_128(x)

#endif  // MCUCORE_SRC_PROGMEM_STRING_DATA_H_
