#include "print/print_to_buffer.h"

#include <string.h>  // For memcpy

#include "log/log.h"

namespace mcucore {

PrintToBuffer::PrintToBuffer(uint8_t* buffer, size_t buffer_size)
    : buffer_(buffer), buffer_size_(buffer_size), bytes_written_(0) {
  MCU_DCHECK_NE(buffer_, nullptr);
  MCU_DCHECK_GT(buffer_size_, 0);
}

PrintToBuffer::~PrintToBuffer() {}

size_t PrintToBuffer::write(uint8_t b) {
  MCU_DCHECK_LE(bytes_written_, buffer_size_);
  if (OkToWrite()) {
    if (buffer_size_ > bytes_written_) {
      buffer_[bytes_written_++] = b;
      return 1;
    }
    // The buffer is full, but we haven't yet overflowed the buffer.
    if (EmptyBuffer()) {
      MCU_DCHECK_EQ(bytes_written_, 0);
      buffer_[0] = b;
      bytes_written_ = 1;
      return 1;
    }
    // Darn. This is the first byte of overflow the buffer.
    setWriteError();
  }
  return 0;
}

size_t PrintToBuffer::write(const uint8_t* input, const size_t size) {
  MCU_DCHECK_LE(bytes_written_, buffer_size_);
  if (!OkToWrite() || size == 0) {
    return 0;
  }

  MCU_VLOG(3) << MCU_NAME_VAL(input) << MCU_NAME_VAL(size);

  size_t input_size = size;

  if (bytes_written_ > 0) {
    if (buffer_size_ > bytes_written_) {
      // Non-empty buffer.
      const size_t available = buffer_size_ - bytes_written_;
      MCU_VLOG(3) << MCU_NAME_VAL(available);
      if (available >= size) {
        // Enough room for all of the input.
        memcpy(buffer_ + bytes_written_, input, size);
        bytes_written_ += size;
        return size;
      }
      // More data than will fit. Fill it as much as we can, then deal with the
      // remainder.
      memcpy(buffer_ + bytes_written_, input, available);
      input_size -= available;
      input += available;
      MCU_DCHECK_GT(input_size, 0);
      MCU_DCHECK_EQ(bytes_written_ + available, buffer_size_);
      bytes_written_ = buffer_size_;
    }
    // buffer_ is full, but we have more data to write.
    MCU_DCHECK_EQ(buffer_size_, bytes_written_);
    if (!EmptyBuffer()) {
      return 0;
    }
  }

  // buffer_ should be completely empty.
  MCU_DCHECK_EQ(bytes_written_, 0);

  // If the remaining input is *bigger* than buffer_, attempt to flush it
  // immediately. We don't do so when it would exactly fill buffer_ so that this
  // class can be used without a subclass, and make full use of all of the
  // provided buffer space.
  if (input_size > buffer_size_) {
    if (FlushData(input, input_size)) {
      return size;
    }
    setWriteError();
    // We don't return on the number of bytes that *might* have been written
    // successfully.
    return 0;
  }

  // buffer_ should be completely empty, and be bigger than the input.
  MCU_DCHECK_EQ(bytes_written_, 0);
  MCU_DCHECK_GE(buffer_size_, input_size);

  // Copy the remaining input into buffer_.
  memcpy(buffer_, input, input_size);
  bytes_written_ = input_size;
  return size;
}

void PrintToBuffer::flush() { EmptyBuffer(); }

int PrintToBuffer::availableForWrite() {
  if (OkToWrite() && buffer_size_ > bytes_written_) {
    return buffer_size_ - bytes_written_;
  } else {
    return 0;
  }
}

size_t PrintToBuffer::data_size() const {
  return min(bytes_written_, buffer_size_);
}

void PrintToBuffer::Reset() {
  bytes_written_ = 0;
  clearWriteError();
}

StringView PrintToBuffer::ToStringView() const {
  return StringView(chars(), data_size());
}

bool PrintToBuffer::FlushData(const uint8_t*, size_t) { return false; }

bool PrintToBuffer::EmptyBuffer() {
  if (OkToWrite()) {
    if (empty()) {
      return true;
    } else if (FlushData(buffer_, data_size())) {
      Reset();
      return true;
    }
    // The caller will need to set bytes_written_ as appropriate to reflect that
    // an overflow has occurred.
    setWriteError();
  }
  return false;
}

}  // namespace mcucore
