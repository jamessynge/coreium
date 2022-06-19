#ifndef MCUCORE_SRC_TINY_STRING_H_
#define MCUCORE_SRC_TINY_STRING_H_

// TinyString is a string with a fixed maximum size and no NUL termination.
// It can be used where we know the maximum size needed for a small, variable
// length string; for example, storing the values of some parameters that might
// come in requests.
//
// Author: james.synge@gmail.com

#include <string.h>  // For memmove

#include "logging.h"
#include "mcucore_platform.h"

namespace mcucore {

// The maximum maximum_size (N) is 255.
template <uint8_t N>
class TinyString {
 public:
  using size_type = uint8_t;

  void Clear() { size_ = 0; }

  // Set the string by copying the 'size' characters from 'from'.
  // Returns false if size is too large.
  bool Set(const char* from, size_type size) {
    MCU_DCHECK_LE(size, N) << MCU_FLASHSTR("Too big");
    Clear();
    if (size > N) {
      return false;
    }
    memmove(data_, from, size);
    size_ = size;
    return true;
  }

  // Set the size explicitly. This allows for data to be copied into here from
  // PROGMEM by a caller, without having to add PROGMEM support here.
  // Returns false if size is too large.
  bool set_size(size_type size) {
    MCU_DCHECK_LE(size, N) << MCU_FLASHSTR("Too big");
    if (size > N) {
      return false;
    }
    size_ = size;
    return true;
  }

  char* data() { return data_; }
  const char* data() const { return data_; }

  size_type size() const { return size_; }
  static constexpr size_type maximum_size() { return N; }
  bool empty() const { return size_ == 0; }

  // Print the string to the provided Print instance.
  size_t printTo(Print& out) const { return out.write(data_, size_); }

 private:
  uint8_t size_{0};
  char data_[N];
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_TINY_STRING_H_
