#ifndef MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_DECODE_CHUNK_ENCODED_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_DECODE_CHUNK_ENCODED_H_

// To aid in testing HTTP/1.1 request or response bodies that are chunk transfer
// encoded, provides a function for decoding such a body. For more info, see:
//
//    https://en.wikipedia.org/wiki/Chunked_transfer_encoding
//
// DecodeChunkEncoded assumes that the entirety of the body is available. For
// further simplicity, we assume that sizeof(char) == sizeof(uint8_t), and treat
// the body (encoded and decoded) as a string rather than as an array of bytes.

#include <string>
#include <string_view>
#include <utility>

#include "absl/status/statusor.h"

namespace mcucore {
namespace test {

// Returns a string containing the concatenated data found in `encoded`, which
// must be a chunk transfer encoded string, and a string containing any data
// that remains after the last chunk (whose size is 0, which is not true for any
// other chunk). If the input is malformed (including not having an empty last
// chunk), then an error is returned.
absl::StatusOr<std::pair<std::string, std::string>> DecodeChunkEncoded(
    std::string_view encoded);

// Helper methods, exposed for testing.

// Given a string_view that starts at the beginning of a chunk, removes the text
// that represents the chunk size, and the size terminator, "\r\n". Returns the
// size and the remainder of `encoded`. If the input is malformed, then an error
// is returned.
absl::StatusOr<std::pair<size_t, std::string_view>> DecodeChunkSize(
    std::string_view encoded);

// Given a string_view that starts with at least one full chunk, returns a view
// of the data of the chunk, and a view of the content following the full chunk.
// If the input is malformed, then an error is returned.
absl::StatusOr<std::pair<std::string_view, std::string_view>> DecodeOneChunk(
    std::string_view encoded);

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_DECODE_CHUNK_ENCODED_H_
