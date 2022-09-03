#ifndef MCUCORE_SRC_PRINT_PRINT_CHUNK_ENCODED_H_
#define MCUCORE_SRC_PRINT_PRINT_CHUNK_ENCODED_H_

// Support for printing using the "chunked" Transfer-Encoding of HTTP/1.1.
// This eliminates the need to pre-compute the size of the content body, which
// may be more expensive than the buffering implied by using this class. TBD.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "print/print_to_buffer.h"

namespace mcucore {

class PrintChunkEncoded : public PrintToBuffer {
 public:
  // Buffer HTTP/1.1 body bytes in the array `buffer[buffer_size]`, and flush as
  // chunk encoded to `out`.
  PrintChunkEncoded(uint8_t* buffer, size_t buffer_size, Print& out);

  // Print to a buffer (array) whose size is known at compile time.
  template <size_t N>
  PrintChunkEncoded(uint8_t (&buffer)[N], Print& out)
      : PrintChunkEncoded(buffer, N, out) {}

  // The destructor flushes any remaining data as the final non-empty chunk, and
  // then writes the empty last chunk.
  ~PrintChunkEncoded() override;

 private:
  bool FlushData(const uint8_t* data, const size_t size) override;

  Print& out_;
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_PRINT_PRINT_CHUNK_ENCODED_H_
