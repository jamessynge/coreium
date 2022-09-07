#ifndef MCUCORE_SRC_MISC_PREPROC_H_
#define MCUCORE_SRC_MISC_PREPROC_H_

// Preprocessor helpers.

// MCU_PP_CONCAT_TOKENS concatenates two preprocessor tokens together, producing
// a new token. This is used to ensure that macro arguments are expanded before
// they are concatenated to produce a new token.

#define MCU_PP_CONCAT_TOKENS(token1, token2) \
  MCU_PP_CONCAT_TOKENS_INNER_(token1, token2)
#define MCU_PP_CONCAT_TOKENS_INNER_(token1, token2) token1##token2

#define MCU_PP_CONCAT_3_TOKENS(token1, token2, token3) \
  MCU_PP_CONCAT_TOKENS(MCU_PP_CONCAT_TOKENS(token1, token2), token3)

// MCU_PP_UNIQUE_NAME generates a token (symbol) that starts with `base_name`,
// and has a numeric suffix that is (usually) unique. If __COUNTER__, a
// non-standard preprocessor feature is available, we'll use it, else we'll
// fallback to using __LINE__ as the unique-ifying value. This produces a unique
// symbol starting with `base_name` unless `MCU_PP_UNIQUE_NAME(base_name)` is
// used in these situations:
//
// a) More than one compilation unit uses it to generate externally visible
//    names with the same `base_name`, and with same value of __COUNTER__ or
//    __LINE__ happens to be used to unique-ify `base_name`.
//
// b) We need to generate more than one such symbol on a single line (e.g. a
//    single macro uses `MCU_PP_UNIQUE_NAME(base_name)` more than once). The
//    work around for this is to use MCU_PP_CONCAT_TOKENS to generate a unique
//    base_name for each distinct symbol that is required.
//
// c) More than one file among those that are compiled together in a single
//    compilation unit (i.e. a source file and the files it includes) happen use
//    `MCU_PP_UNIQUE_NAME(base_name)`, on the same line number within their
//    files, and __COUNTER__ isn't available.

#if defined(__clang__) || defined(__GNUC__) || defined(_MSVC_LANG)
#define MCU_PP_UNIQUE_NAME(base_name) \
  MCU_PP_CONCAT_TOKENS(base_name, __COUNTER__)
#else
#define MCU_PP_UNIQUE_NAME(base_name) MCU_PP_CONCAT_TOKENS(base_name, __LINE__)
#endif

#endif  // MCUCORE_SRC_MISC_PREPROC_H_
