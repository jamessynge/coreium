#include "extras/test_tools/http1/decode_chunk_encoded.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "util/task/status_macros.h"

namespace mcucore {
namespace test {
namespace {
absl::StatusOr<std::string_view> SkipCrLf(const std::string_view str) {
  if (str.size() < 2) {
    return absl::InvalidArgumentError(
        absl::StrCat("String too short (", str.size(), ") for CRLF: ", str));
  } else if (str.substr(0, 2) != "\r\n") {
    return absl::InvalidArgumentError(absl::StrCat("Expected CRLF: ", str));
  }
  return str.substr(2);
}
}  // namespace

absl::StatusOr<std::pair<size_t, std::string_view>> DecodeChunkSize(
    const std::string_view encoded) {
  if (encoded.empty()) {
    return absl::InvalidArgumentError("Expected chunk, got empty string.");
  }
  const auto pos = encoded.find_first_not_of("0123456789ABCDEFabcdef");
  if (pos == std::string_view::npos) {
    return absl::InvalidArgumentError(
        absl::StrCat("Expected chunk size to end: ", encoded));
  } else if (pos == 0) {
    return absl::InvalidArgumentError(
        absl::StrCat("Expected chunk size: ", encoded));
  }
  ASSIGN_OR_RETURN(const auto rest, SkipCrLf(encoded.substr(pos)));
  const auto size_str = encoded.substr(0, pos);
  size_t size;
  if (!absl::SimpleHexAtoi(size_str, &size)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Failed to convert with SimpleHexAtoi: ", size_str));
  }
  return std::make_pair(size, rest);
}

absl::StatusOr<std::pair<std::string_view, std::string_view>> DecodeOneChunk(
    const std::string_view encoded) {
  ASSIGN_OR_RETURN(const auto size_and_remainder, DecodeChunkSize(encoded));
  const auto [size, remainder] = size_and_remainder;
  if (size > remainder.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Expected chunk size too long (", size, " > ",
                     remainder.size(), "): ", remainder));
  }
  const auto data = remainder.substr(0, size);
  ASSIGN_OR_RETURN(const auto rest, SkipCrLf(remainder.substr(size)));
  return std::make_pair(data, rest);
}

absl::StatusOr<std::pair<std::string, std::string>> DecodeChunkEncoded(
    std::string_view encoded) {
  std::string result;
  while (!encoded.empty()) {
    ASSIGN_OR_RETURN(const auto chunk_data_and_rest, DecodeOneChunk(encoded));
    const auto [chunk_data, rest] = chunk_data_and_rest;
    result += chunk_data;
    if (chunk_data.empty()) {
      return std::make_pair(result, std::string(rest));
    }
    encoded = rest;
  }
  return absl::InvalidArgumentError(
      absl::StrCat("Found no last chunk, after decoding: ", result));
}

}  // namespace test
}  // namespace mcucore
