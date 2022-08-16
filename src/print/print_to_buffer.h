#ifndef MCUCORE_SRC_PRINT_PRINT_TO_BUFFER_H_
#define MCUCORE_SRC_PRINT_PRINT_TO_BUFFER_H_

// Support for printing to a fixed size byte buffer, with overflow detection.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "strings/string_view.h"
#include "strings/tiny_string.h"

namespace mcucore {

class PrintToBuffer : public Print {
  static_assert(sizeof(uint8_t) == sizeof(char));

 public:
  PrintToBuffer(uint8_t* buffer, size_t buffer_size);
  PrintToBuffer(char* buffer, size_t buffer_size)
      : PrintToBuffer(reinterpret_cast<uint8_t*>(buffer), buffer_size) {}

  // Print to a buffer (array) whose size is known at compile time.
  template <size_t N>
  explicit PrintToBuffer(uint8_t (&buffer)[N]) : PrintToBuffer(buffer, N) {}
  template <size_t N>
  explicit PrintToBuffer(char (&buffer)[N]) : PrintToBuffer(buffer, N) {}

  // Appends to the TinyString *after* any text that is already in the string.
  // Note that this will NOT update the size of the TinyString, so you'll need
  // to do something like this to update it after writing to the PrintToBuffer:
  //
  //    tiny_str.set_size(tiny_str.size() + print2buffer.data_size());
  //
  template <uint8_t N>
  explicit PrintToBuffer(TinyString<N>& buffer)
      : PrintToBuffer(buffer.data() + buffer.size(),
                      buffer.maximum_size() - buffer.size()) {}

  virtual ~PrintToBuffer();  // NOLINT

  // Append `b` to the buffer.
  size_t write(uint8_t b) override;

  // Append `size` bytes to the buffer.
  size_t write(const uint8_t* buffer, size_t size) override;

  size_t buffer_size() const { return buffer_size_; }

  // Number of written bytes stored in the buffer. Will not exceed
  // buffer_size().
  size_t data_size() const;

  // Returns the number of bytes written since last Reset(), or construction.
  // may exceed buffer_size().
  size_t bytes_written() const { return bytes_written_; }

  // Returns true if more bytes have been written than can be stored.
  bool has_buffer_overflow() const;

  // Reset so that we can store new data.
  void Reset() { bytes_written_ = 0; }

  uint8_t* buffer() const { return buffer_; }
  char* chars() const { return reinterpret_cast<char*>(buffer_); }

  // Return a StringView of the stored chars (i.e. the first data_size()
  // characters in the buffer).
  StringView ToStringView() const;

 private:
  // The buffer into which bytes are written.
  uint8_t* const buffer_;

  // Size of the buffer provided when constructed.
  const size_t buffer_size_;

  // Number of bytes written; may exceed buffer_size_.
  size_t bytes_written_;
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_PRINT_PRINT_TO_BUFFER_H_
