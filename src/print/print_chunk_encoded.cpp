#include "print/print_chunk_encoded.h"

#include "print/print_to_buffer.h"

namespace mcucore {
namespace {
void PrintCrLf(Print& out) {
  out.print('\r');
  out.print('\n');
}
}  // namespace

PrintChunkEncoded::PrintChunkEncoded(uint8_t* buffer, size_t buffer_size,
                                     Print& out)
    : PrintToBuffer(buffer, buffer_size), out_(out) {}

PrintChunkEncoded::~PrintChunkEncoded() {
  flush();
  if (OkToWrite()) {
    MCU_DCHECK_EQ(data_size(), 0);
    // Write the last (empty) chunk, which tells the receiver that they've read
    // everything.
    out_.print('0');
    PrintCrLf(out_);

    // Print the terminating CRLF.
    PrintCrLf(out_);
  }
}

bool PrintChunkEncoded::FlushData(const uint8_t* const data,
                                  const size_t size) {
  // Write the size as a hexadecimal number.
  MCU_DCHECK_GT(size, 0);
  out_.print(size, 16);
  PrintCrLf(out_);

  // Write the data of the chunk.
  out_.write(data, size);
  PrintCrLf(out_);

  // Empty the buffer.
  Reset();
  return true;
}

}  // namespace mcucore
