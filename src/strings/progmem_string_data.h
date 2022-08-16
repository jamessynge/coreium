#ifndef MCUCORE_SRC_STRINGS_PROGMEM_STRING_DATA_H_
#define MCUCORE_SRC_STRINGS_PROGMEM_STRING_DATA_H_

// Provides macros for storing string literals in program memory (PROGMEM)
// rather than RAM, when compiled with avr-gcc for the AVR line of processors.
//
// At root these macros use the template struct ProgmemStringData to store the
// value in a static field of a full specialization of the template. This
// enables the linker to collapse multiple occurrences of the same string
// literal into a single array in PROGMEM; this isn't true when using the
// Arduino defined F(string_literal) macro, where every occurrence in a single
// file is stored separately.
//
// (We use MCU_FLASHSTR(str) (or FLASHSTR(str) if not including this file),
// rather than F(str) due to conflicts caused by Arduino's F as the name for
// that macro. While macros should generally be avoided, they are nonetheless
// useful, so when they are defined they should have names that are not very
// likely to collide with choices made by others. Sigh.)
//
// MCU_PSV(string_literal) expands to a ProgmemStringView instance with
// string_literal as the value it views.
//
// MCU_FLASHSTR(string_literal) expands to a `const __FlashStringHelper*`
// pointer value, just as F(string_literal) does, but without the wasted storage
// when the string literal appears multiple times.
//
// MCU_BASENAME(file_path_string_literal) is like MCU_FLASHSTR, but trims off
// any characters to the left of the rightmost slash (forward or backward) in
// file_path_literal; i.e. it expands to a `const __FlashStringHelper*` that
// points to the file name in the file_path, but without the path of the
// directory containing that file. This exists to support logging.h, which has
// support for including the location (file name and line number) at which a
// message was logged.
//
// -----------------------------------------------------------------------------
//
// The approach of using template metaprogramming and type deduction is inspired
// by George Makrydakis https://github.com/irrequietus/typestring. typestring.hh
// expands a string literal passed to a macro function into a list of char
// values which is used as a template parameter pack; template metaprogramming
// is used to iterate through the characters looking for the string terminating
// null character, though recursive type deduction is used to perform that
// iteration. This works fine for short strings, but with longer strings (e.g.
// above 128) the recursive type deduction becomes rather slow, and can exceed
// the the stack size supported by the compiler.
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
// 3) Only the fragment containing the last character AND the terminating null
//    is processed in a special fashion to trim off the terminating null and the
//    following padding, which is also made up of null characters. Fragments
//    containing string characters but not the terminating null, and fragments
//    containing only null characters (starting with the terminating null), are
//    processed as a whole unit, without needing to process each character.
//
// 4) Recursion is performed in a binary tree fashion on adjacent fragments or
//    sequences of adjacent fragments, rather than as just a character at a time
//    (ala iteration). This means that the fragments 1 and 2 are concatenated
//    together into fragment A, fragments 3 and 4 are concatenated together into
//    fragment B, and finally fragments A and B are concatenated together into
//    fragment C. This limits the depth of the compiler stack required to
//    perform type deduction.
//
// Note that character-at-a-time recursion is retained in the implementation of
// finding slashes within the 16 character string fragments, but after that the
// binary tree recursion method is used, which addresses the difficulties with
// long __FILE__ strings.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"

namespace mcucore {
namespace progmem_string_data {

// Get the Nth char from a string literal of length M, where that length
// includes the trailing null character at index M-1. This is used to produce
// the comma separated lists of chars that make up a literal string. If N is >
// M, the trailing null character is returned.
template <int N, int M>
constexpr char GetNthCharOfM(char const (&c)[M]) {
  return c[N < M ? N : M - 1];
}

// Full specializations of this template define static, constexpr storage for
// string literals. Because all full specializations for the same string literal
// produce exactly the same type, the compiler and linker will treat multiple
// occurrences of the type as the same, and thus will combine multiple
// occurrences so that only a single copy of the string literal is stored in the
// program (i.e. we don't waste flash memory with multiple copies).
template <char... C>
struct ProgmemStringData final {
  // We add a trailing null character to the array so that we can interpret
  // kData as a __FlashStringHelper instance (see MCU_FLASHSTR); Arduino's
  // Print::print(const __FlashStringHelper*) needs the string to be NUL
  // terminated so that it knows when it has found the end of the string.
  using ArrayType = char[1 + sizeof...(C)];
  static constexpr char const kData[1 + sizeof...(C)] AVR_PROGMEM = {C..., 0};

  // Return the array.
  constexpr const ArrayType& progmem_char_array() const { return kData; }

  // Return the number of chars in the string, not including the terminating
  // NUL.
  static constexpr int size() { return sizeof...(C); }
};

// 'Define' the storage for the kData array, though in fact that won't happen
// until the template is instantiated.
template <char... C>
constexpr char const
    ProgmemStringData<C...>::kData[1 + sizeof...(C)] AVR_PROGMEM;

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

// StringFragment is a template struct whose parameters capture a sequence of
// zero or more characters from a string literal.
template <char... C>
struct StringFragment final {};

// DiscardCount is a template struct whose parameter captures the number of
// trailing null characters to be discarded from the last (partial) string
// fragment.
template <int N>
struct DiscardCount final {};

// LengthCheck is a template struct whose parameter captures whether or not the
// length of the string literal was supported by the macro used (true) or not
// (false). This is used to produce a compile time error if too long a string is
// passed to one of the macros.
template <bool LengthOK>
struct LengthCheck final {};

// "Forward" declaration of an undefined type. If this appears in a compiler
// error message for the expansion of MCU_FLASHSTR_nnn, or a related macro, it
// means that the string literal is too long (> nnn).
class StringLiteralIsTooLong;

////////////////////////////////////////////////////////////////////////////////
// KeepLiteral has specializations for each count of trailing null characters to
// be dicarded (15 through 1). If it is necessary to edit or replace these, see
// the script extras/dev_tools/make_keep_literal.py.

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

////////////////////////////////////////////////////////////////////////////////
// DiscardAfterLiteral is used to decide how to translate a Phase1StringFragment
// into a StringFragment whose char... parameter pack contains only characters
// of the string, and not the null termination, nor any trailing padding.

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

// Concatenate (adjacent) string fragments.
template <char... X, char... Y>
auto Concat(StringFragment<X...>, StringFragment<Y...>)
    -> StringFragment<X..., Y...>;

// If the macro used (e.g. PSV_nnn) supports strings as long as that
// provided, then ProvideStorage's return type will have a static array with
// the string in it.
template <char... C>
auto ProvideStorage(LengthCheck<true>, StringFragment<C...>)
    -> ProgmemStringData<C...>;

// Else if the literal is too long for the expension macro used, then
// ProvideStorage's return type will be StringLiteralIsTooLong, an undefined
// type whose name indicates what the problem is.
template <char... C>
auto ProvideStorage(LengthCheck<false>, StringFragment<C...>)
    -> StringLiteralIsTooLong;

////////////////////////////////////////////////////////////////////////////////
// Support for computing the basename of a file path, which mostly means
// __FILE__. KeepBasename searches each StringFragment for slashes, retaining
// only characters after the rightmost slash, and also indicating whether we
// have found such a slash. Specializations of Concat support concatenating the
// results of KeepBasename, again retaining only characters after the rightmost
// slash.

// Represents a string fragment while we're removing slashes and preceding
// characters. FoundSlash is true if the evaluation that produced the type found
// a slash.
template <bool FoundSlash, char... C>
struct PathFragment final {};

// KeepBasename is a set of template function declarations for removing slashes
// and preceding characters.

template <bool FoundSlash, char... C>
auto KeepBasename(PathFragment<FoundSlash, C...>)  // All done.
    -> PathFragment<FoundSlash, C...>;

template <bool FoundSlash, char A, char... X, char... Y>
auto KeepBasename(PathFragment<FoundSlash, X...>, StringFragment<A>,
                  StringFragment<Y>...)
    -> decltype(KeepBasename(PathFragment<FoundSlash, X..., A>(),
                             StringFragment<Y>()...));

template <bool FoundSlash, char... X, char... Y>
auto KeepBasename(PathFragment<FoundSlash, X...>, StringFragment<'/'>,
                  StringFragment<Y>...)
    -> decltype(KeepBasename(PathFragment<true>(), StringFragment<Y>()...));

template <bool FoundSlash, char... X, char... Y>
auto KeepBasename(PathFragment<FoundSlash, X...>, StringFragment<'\\'>,
                  StringFragment<Y>...)
    -> decltype(KeepBasename(PathFragment<true>(), StringFragment<Y>()...));

// Entry point for trimming a fragment of a file path to just those characters
// after any slashes found in the fragment. The input is the result of
// DiscardAfterLiteral.
template <char... C>
auto KeepBasename(StringFragment<C...>)
    -> decltype(KeepBasename(PathFragment<false>(), StringFragment<C>()...));

// Concatenate path fragments, keeping only the characters after the right most
// slash.

// Matches a pair of path fragments where the right-hand fragment does not have
// a slash in it; as a result we keep the characters in both fragments, and the
// left-hand fragment determines whether the return type denotes a fragment in
// which a slash was found.
template <bool FoundSlash, char... X, char... Y>
auto Concat(PathFragment<FoundSlash, X...>, PathFragment<false, Y...>)
    -> PathFragment<FoundSlash, X..., Y...>;

// Matches a pair of path fragments where the right-hand fragment had a slash in
// it; as a result we don't need the left-hand fragment at all.
template <bool FoundSlash, char... X, char... Y>
auto Concat(PathFragment<FoundSlash, X...>, PathFragment<true, Y...>)
    -> PathFragment<true, Y...>;

// Another specialization of ProvideStorage, which transforms a PathFragment
// back into a StringFragment and the uses the previously defined
// specializations of ProvideStorage.
template <bool LengthOk, bool FoundSlash, char... C>
auto ProvideStorage(LengthCheck<LengthOk>, PathFragment<FoundSlash, C...>)
    -> decltype(ProvideStorage(LengthCheck<LengthOk>(),
                               StringFragment<C...>()));

}  // namespace progmem_string_data
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
#define _PSD_NS ::mcucore::progmem_string_data

// Expands to the n-th character of x; if n is greater than the size of x, then
// expands to the null character.
#define _PSD_GET_NTH_CHAR(n, x) _PSD_NS::GetNthCharOfM<0x##n>(x)

// Value used to detect when the string literal is longer than is supported by
// the macro used to decompose the string literal into characters. The value is
// used to select the appropriate specialization of ProvideStorage.
#define _PSD_LENGTH_CHECK(x, max_chars) \
  (_PSD_NS::LengthCheck<((sizeof x) - 1 <= max_chars)>())

// Support for the working with individual string fragments of 16 characters.

// Expands to a Phase1StringFragment type for the portion of string |x| starting
// at offset 0x##n##0, where n is zero or more hexadecimal digits; there is no
// leading 0x before the digits in |n|. Any character offset that is beyond the
// last character of |x| is expanded to the null character.
#define _PSD_PHASE1_TYPE(n, x)                                                \
  _PSD_NS::Phase1StringFragment<                                              \
      /* BeforeLastChar */ ((sizeof x) >= 16 && 0x##n##F < ((sizeof x) - 1)), \
      /* CharsToDiscard */ (16 - (((sizeof x) - 1) % 16)),                    \
      /* AfterLastChar */ (0x##n##0 >= ((sizeof x) - 1)),                     \
      _PSD_GET_NTH_CHAR(n##0, x), _PSD_GET_NTH_CHAR(n##1, x),                 \
      _PSD_GET_NTH_CHAR(n##2, x), _PSD_GET_NTH_CHAR(n##3, x),                 \
      _PSD_GET_NTH_CHAR(n##4, x), _PSD_GET_NTH_CHAR(n##5, x),                 \
      _PSD_GET_NTH_CHAR(n##6, x), _PSD_GET_NTH_CHAR(n##7, x),                 \
      _PSD_GET_NTH_CHAR(n##8, x), _PSD_GET_NTH_CHAR(n##9, x),                 \
      _PSD_GET_NTH_CHAR(n##A, x), _PSD_GET_NTH_CHAR(n##B, x),                 \
      _PSD_GET_NTH_CHAR(n##C, x), _PSD_GET_NTH_CHAR(n##D, x),                 \
      _PSD_GET_NTH_CHAR(n##E, x), _PSD_GET_NTH_CHAR(n##F, x)>

// Expands to a string fragment type without trailing nulls.
#define _PSD_STRFRAG_TYPE(n, x) \
  decltype(_PSD_NS::DiscardAfterLiteral(_PSD_PHASE1_TYPE(n, x)()))

// Expands to a path fragment type without trailing nulls, and without any
// slashes or characters preceding such slashes.
#define _PSD_PATHFRAG_TYPE(n, x) \
  decltype(_PSD_NS::KeepBasename(_PSD_STRFRAG_TYPE(n, x)()))

////////////////////////////////////////////////////////////////////////////////
// Expands to the type of two (adjacent) string fragments, t1() and t2(), being
// concatenated together.
#define _PSD_CONCAT_TYPE(t1, t2) decltype(_PSD_NS::Concat(t1(), t2()))

// Concatenates two preprocessor tokens together. This is used below to ensure
// that two macro arguments are expanded before they are concatenated to produce
// a new token.
#define _PSD_CONCAT_TOKENS(token1, token2) token1##token2

// Expands to the type from concatenating the types of two string fragments of
// 16 characters starting at 0x##hex0##hex1##0 and at 0x##hex0##hex2##0.
#define _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, hex0, hex1, hex2, x) \
  _PSD_CONCAT_TYPE(fragtype(_PSD_CONCAT_TOKENS(hex0, hex1), x),   \
                   fragtype(_PSD_CONCAT_TOKENS(hex0, hex2), x))

// Expands to the type from concatenating the types of two path fragments of 16
// characters starting at 0x##hex0##hex1##0 and at 0x##hex0##hex2##0.
#define _PSD_CONCAT_PATHFRAG_TYPE(hex0, hex1, hex2, x)                    \
  _PSD_CONCAT_TYPE(_PSD_PATHFRAG_TYPE(_PSD_CONCAT_TOKENS(hex0, hex1), x), \
                   _PSD_PATHFRAG_TYPE(_PSD_CONCAT_TOKENS(hex0, hex2), x))

////////////////////////////////////////////////////////////////////////////////
// Macros for producing StringFragment, PathFragment and ProgmemStringData types
// that support string literals whose sizes do not exceed the specified number
// of chars.

#define _MAKE_PSD_TYPE_nnn(nnn, fragtype, x) \
  decltype(_PSD_NS::ProvideStorage(          \
      _PSD_LENGTH_CHECK(x, nnn), _PSD_CONCAT_##nnn##_TYPE(fragtype, 0, x)()))

/* 2^5 = 32 */
#define _PSD_CONCAT_32_TYPE(fragtype, n, x) \
  _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 0, 1, x)

#define _PSD_TYPE_32(fragtype, x) _MAKE_PSD_TYPE_nnn(32, fragtype, x)

/* 2^6 = 64 */
#define _PSD_CONCAT_64_TYPE(fragtype, n, x)                          \
  _PSD_CONCAT_TYPE(_PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 0, 1, x), \
                   _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 2, 3, x))

#define _PSD_TYPE_64(fragtype, x) _MAKE_PSD_TYPE_nnn(64, fragtype, x)

/* 2^7 = 128 */
#define _PSD_CONCAT_128_TYPE(fragtype, n, x)                              \
  _PSD_CONCAT_TYPE(                                                       \
      _PSD_CONCAT_TYPE(_PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 0, 1, x),  \
                       _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 2, 3, x)), \
      _PSD_CONCAT_TYPE(_PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 4, 5, x),  \
                       _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 6, 7, x)))

#define _PSD_TYPE_128(fragtype, x) _MAKE_PSD_TYPE_nnn(128, fragtype, x)

// Special case: length 255, for those cases below where the string length must
// be encoded in a uint8.
#define _PSD_TYPE_255(fragtype, x)                            \
  decltype(_PSD_NS::ProvideStorage(_PSD_LENGTH_CHECK(x, 255), \
                                   _PSD_CONCAT_256_TYPE(fragtype, 0, x)()))

/* 2^8 = 256 */
#define _PSD_CONCAT_256_TYPE(fragtype, n, x)                                   \
  _PSD_CONCAT_TYPE(                                                            \
      _PSD_CONCAT_TYPE(                                                        \
          _PSD_CONCAT_TYPE(_PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 0, 1, x),   \
                           _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 2, 3, x)),  \
          _PSD_CONCAT_TYPE(_PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 4, 5, x),   \
                           _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 6, 7, x))), \
      _PSD_CONCAT_TYPE(                                                        \
          _PSD_CONCAT_TYPE(_PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, 8, 9, x),   \
                           _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, A, B, x)),  \
          _PSD_CONCAT_TYPE(_PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, C, D, x),   \
                           _PSD_CONCAT_FRAGMENTS_TYPE(fragtype, n, E, F, x))))

#define _PSD_TYPE_256(fragtype, x) _MAKE_PSD_TYPE_nnn(256, fragtype, x)

/* 2^9 = 512 */
#define _PSD_CONCAT_512_TYPE(fragtype, n, x)                       \
  _PSD_CONCAT_TYPE(                                                \
      _PSD_CONCAT_256_TYPE(fragtype, _PSD_CONCAT_TOKENS(n, 0), x), \
      _PSD_CONCAT_256_TYPE(fragtype, _PSD_CONCAT_TOKENS(n, 1), x))

#define _PSD_TYPE_512(fragtype, x) _MAKE_PSD_TYPE_nnn(512, fragtype, x)

/* 2^10 = 1024 */
#define _PSD_CONCAT_1024_TYPE(fragtype, n, x)                           \
  _PSD_CONCAT_TYPE(                                                     \
      _PSD_CONCAT_TYPE(                                                 \
          _PSD_CONCAT_256_TYPE(fragtype, _PSD_CONCAT_TOKENS(n, 0), x),  \
          _PSD_CONCAT_256_TYPE(fragtype, _PSD_CONCAT_TOKENS(n, 1), x)), \
      _PSD_CONCAT_TYPE(                                                 \
          _PSD_CONCAT_256_TYPE(fragtype, _PSD_CONCAT_TOKENS(n, 2), x),  \
          _PSD_CONCAT_256_TYPE(fragtype, _PSD_CONCAT_TOKENS(n, 3), x)))

#define _PSD_TYPE_1024(fragtype, x) _MAKE_PSD_TYPE_nnn(1024, fragtype, x)

////////////////////////////////////////////////////////////////////////////////
// We define below macros MCU_PSD_nnn (PSD==ProgmemStringData),
// MCU_FLASHSTR_nnn, and MCU_BASENAME_nnn for various values of nnn, which
// represents the maximum length of string literal (not including the
// terminating null character) supported by the macro. These produce *values*
// that can be printed or otherwise operated upon at runtime.

// Max length 32:

#define MCU_PSD_32(x) (_PSD_TYPE_32(_PSD_STRFRAG_TYPE, x)())

#define MCU_FLASHSTR_32(x)                       \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_32(_PSD_STRFRAG_TYPE, x)::kData))

#define MCU_BASENAME_32(x)                       \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_32(_PSD_PATHFRAG_TYPE, x)::kData))

// Max length 64 (not including trailing NUL).

#define MCU_PSD_64(x) (_PSD_TYPE_64(_PSD_STRFRAG_TYPE, x)())

#define MCU_FLASHSTR_64(x)                       \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_64(_PSD_STRFRAG_TYPE, x)::kData))

#define MCU_BASENAME_64(x)                       \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_64(_PSD_PATHFRAG_TYPE, x)::kData))

// Max length 128 (not including trailing NUL).

#define MCU_PSD_128(x) (_PSD_TYPE_128(_PSD_STRFRAG_TYPE, x)())

#define MCU_FLASHSTR_128(x)                      \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_128(_PSD_STRFRAG_TYPE, x)::kData))

#define MCU_BASENAME_128(x)                      \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_128(_PSD_PATHFRAG_TYPE, x)::kData))

// Max length 255 (not including trailing NUL). This is defined to support
// MCU_PSV_255, which is the MCU_PSV* version that handles the longest strings
// that ProgmemStringView can encode (i.e. it uses a single byte for the
// length).

#define MCU_PSD_255(x) (_PSD_TYPE_255(_PSD_STRFRAG_TYPE, x)())

// Max length 256 (not including trailing NUL).

#define MCU_PSD_256(x) (_PSD_TYPE_256(_PSD_STRFRAG_TYPE, x)())

#define MCU_FLASHSTR_256(x)                      \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_256(_PSD_STRFRAG_TYPE, x)::kData))

#define MCU_BASENAME_256(x)                      \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_256(_PSD_PATHFRAG_TYPE, x)::kData))

#define MCU_BASENAME_256_TYPE(x) _PSD_TYPE_256(_PSD_PATHFRAG_TYPE, x)

// Max length 512 (not including trailing NUL). There is no support here for
// ProgmemStringView because it can't support such a long string.

#define MCU_PSD_512(x) (_PSD_TYPE_512(_PSD_STRFRAG_TYPE, x)())

#define MCU_FLASHSTR_512(x)                      \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_512(_PSD_STRFRAG_TYPE, x)::kData))

#define MCU_BASENAME_512(x)                      \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_512(_PSD_PATHFRAG_TYPE, x)::kData))

#define MCU_BASENAME_512_TYPE(x) _PSD_TYPE_512(_PSD_PATHFRAG_TYPE, x)

// Max length 1024 (not including trailing NUL). There is no support here for
// ProgmemStringView because it can't support such a long string.

#define MCU_PSD_1024(x) (_PSD_TYPE_1024(_PSD_STRFRAG_TYPE, x)())

#define MCU_FLASHSTR_1024(x)                     \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_1024(_PSD_STRFRAG_TYPE, x)::kData))

#define MCU_BASENAME_1024(x)                     \
  (reinterpret_cast<const __FlashStringHelper*>( \
      _PSD_TYPE_1024(_PSD_PATHFRAG_TYPE, x)::kData))

#define MCU_BASENAME_1024_TYPE(x) _PSD_TYPE_1024(_PSD_PATHFRAG_TYPE, x)

////////////////////////////////////////////////////////////////////////////////
// Because we expect almost all explicitly defined string literals to be
// relatively short, we define these macros without specific lengths in their
// names for use in most of the code base. Where a longer string literal is
// required, use the appropriate macro defined above whose name specifies the
// next larger size limit.

#define MCU_PSD(x) MCU_PSD_64(x)
#define MCU_FLASHSTR(x) MCU_FLASHSTR_64(x)

// We provide a default MCU_BASENAME for file paths that should work in most
// circumstances. If necessary, change the supported length to a higher power of
// two.
//
// Note that the length of __FILE__ is a function of the file relative file path
// of the file being compiled relative to the root of the build tree, where in
// the file system that build tree is located, and whether or not the compiler
// is including the full path in __FILE__, or just a relative path within the
// build tree. Note that some build systems (e.g. Arduino) copy the input files
// to another location, and thus their original file path may be shorter or
// longer than the one used when they're compiled.

#define MCU_BASENAME(x) MCU_BASENAME_512(x)

#endif  // MCUCORE_SRC_STRINGS_PROGMEM_STRING_DATA_H_
