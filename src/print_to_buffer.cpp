#include "print_to_buffer.h"

#include <string.h>  // For memcpy

#include "string_view.h"

namespace mcucore {

PrintToBuffer::PrintToBuffer(uint8_t* buffer, size_t buffer_size)
    : buffer_(buffer), buffer_size_(buffer_size), bytes_written_(0) {}

PrintToBuffer::~PrintToBuffer() {}

size_t PrintToBuffer::write(uint8_t b) {
  if (buffer_size_ > bytes_written_) {
    buffer_[bytes_written_++] = b;
    return 1;
  }

  setWriteError();
  bytes_written_++;
  return 0;
}

size_t PrintToBuffer::write(const uint8_t* buffer, size_t size) {
  if (buffer_size_ >= bytes_written_ + size) {
    memcpy(buffer_ + bytes_written_, buffer, size);
    bytes_written_ += size;
    return size;
  }

  size_t result = 0;
  for (size_t i = 0; i < size; ++i) {
    result += write(buffer[i]);
  }
  return result;
}

size_t PrintToBuffer::data_size() const {
  return min(bytes_written_, buffer_size_);
}

bool PrintToBuffer::has_buffer_overflow() const {
  return buffer_size_ < bytes_written_;
}

StringView PrintToBuffer::ToStringView() const {
  return StringView(chars(), data_size());
}

}  // namespace mcucore
