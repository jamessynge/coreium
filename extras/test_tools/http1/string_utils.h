#ifndef MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_STRING_UTILS_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_STRING_UTILS_H_

// Helper functions focused on testing the HTTP request decoder.
//
// Author: james.synge@gmail.com

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace mcucore {
namespace test {

// Return all HTTP method names registered with IANA, as of July, 2022.
// https://www.iana.org/assignments/http-methods/http-methods.xhtml
std::vector<std::string> AllRegisteredMethodNames();

// Split the string `full_request` every `n`-th character, returning a vector of
// the parts.
std::vector<std::string> SplitEveryN(const std::string& full_request, size_t n);

// Split `full_request` at many different spacings (e.g. every 1 character,
// every 2 characters, etc.), in support testing the HTTP request decoder to
// handle approximately any possible split point.
std::vector<std::vector<std::string>> GenerateMultipleRequestPartitions(
    const std::string& full_request, size_t max_decode_buffer_size,
    size_t max_literal_match_size);

// Return `buffer` joined with all elements in `partition` starting with element
// `ndx`.
std::string AppendRemainder(const std::string& buffer,
                            const std::vector<std::string>& partition, int ndx);

// Returns all char values except those where function `excluding` returns true.
std::string AllCharsExcept(bool (*excluding)(char c));
std::string AllCharsExcept(int (*excluding)(int c));

// Returns a string with every char value.
std::string EveryChar();

// Returns the character as a percent-encoded string.
std::string PercentEncodeChar(char c);

// Returns a string with every character in `input` percent-encoded.
std::string PercentEncodeAllChars(std::string_view input);

// Returns a set of invalid percent-encoded chars, where one of the two
// characters following the percent is not a hexadecimal digit.
std::vector<std::string> GenerateInvalidPercentEncodedChar();

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_STRING_UTILS_H_
