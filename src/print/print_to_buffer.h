#ifndef MCUCORE_SRC_PRINT_PRINT_TO_BUFFER_H_
#define MCUCORE_SRC_PRINT_PRINT_TO_BUFFER_H_

// Support for printing to a fixed size byte buffer, with an option for draining
// the buffer in a subclass implementation of FlushData; otherwise excess data
// is lost.
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

  // Append `b` to the buffer if no write error has occurred (including
  // overflow), and there is room. If there isn't room FlushData is called,
  // which may make room; if not, then an overflow is recorded and zero is
  // returned. If a write error has occurred, then zero is returned.
  size_t write(uint8_t b) override;

  // Append `size` bytes from `input` to the buffer if no write error has
  // occurred (including overflow), and there is room for the bytes. If there
  // isn't sufficient room, what room is available is filled from the input
  // buffer, and then FlushData is called, which may make room; if FlushData
  // returns true, then `write` continues writing the remainder of the input. If
  // a write error has occurred, then zero is returned.
  size_t write(const uint8_t* input, size_t size) override;

  // Attempts to flush buffer_, assuming it isn't empty, nor has already
  // overflowed.
  void flush() override;

  // Returns the space that is available in the buffer.
  int availableForWrite() override;

  // The Arduino libraries don't declare many methods const, even when they
  // absolutely could be. To make using PrintToBuffer a bit easier to use, I'm
  // declaring const versions of some of those methods.
  int availableForWrite() const {
    return const_cast<PrintToBuffer*>(this)->PrintToBuffer::availableForWrite();
  }
  int getWriteError() const {
    return const_cast<PrintToBuffer*>(this)->Print::getWriteError();
  }

  size_t buffer_size() const { return buffer_size_; }

  bool empty() const { return bytes_written_ == 0; }

  // Number of written bytes stored in the buffer. Will not exceed buffer_size_.
  size_t data_size() const;

  // Returns true if more bytes have been written than can be stored, or if any
  // other write error has been recorded.
  bool HasWriteError() const { return getWriteError() != 0; }

  // Returns true if there is no write error.
  bool OkToWrite() const { return getWriteError() == 0; }

  // Reset so that we can store new data.
  void Reset();

  uint8_t* buffer() const { return buffer_; }
  char* chars() const { return reinterpret_cast<char*>(buffer_); }

  // Return a StringView of the stored chars (i.e. the first data_size()
  // characters in the buffer).
  StringView ToStringView() const;

 private:
  // Called to request that the specified data be flushed to some sink so that
  // it can be considered written, and thus doesn't need to be buffered
  // (anymore).
  //
  // Returns true if all of the data has been written, and false if none of the
  // data has been written; there is no support for partially flushing the
  // buffer. The default implementation returns false, in which case the caller
  // (one of the write methods, or flush) will call setWriteError, and bytes are
  // lost.
  //
  // The data may be from buffer_, or may be from the input buffer passed to
  // `write()`. DO NOT call Reset() from this method; the caller will do that
  // if appropriate.
  //
  // Note that this is NOT called from the dtor because at that point it is too
  // late for a subclass implementation of FlushData to be called.
  virtual bool FlushData(const uint8_t* data, size_t size);

  // Empty the buffer; called as part of writing or flushing. Returns false if
  // an overflow has occurred, or if FlushData returned false. Returns true if
  // able to ensure that the buffer is empty.
  bool EmptyBuffer();

  // The buffer into which bytes are written.
  uint8_t* const buffer_;

  // Size of the buffer provided when constructed.
  const size_t buffer_size_;

  // Number of bytes written; will not exceed buffer_size_.
  size_t bytes_written_;
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_PRINT_PRINT_TO_BUFFER_H_
