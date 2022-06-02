#ifndef MCUCORE_EXTRAS_FUTURES_PROGMEM_POINTER_H_
#define MCUCORE_EXTRAS_FUTURES_PROGMEM_POINTER_H_

// In support of iterating over strings, and possibly other structures, this
// helps to encapsulate some of the challenges of dealing with data stored in
// PROGMEM.
//
// NOTE: So far this only "works" for "near" addresses, i.e. in the first 64KB.
//
// Learn more about writing custom iterators here:
//    https://internalpointers.com/post/writing-custom-iterators-modern-cpp
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"

namespace mcucore {

template <typename T, typename P = PGM_P>
class ProgMemPtr {
 public:
  using value_type = T;
  using reference_type = value_type&;
  using pointer_type = value_type*;
  using unit_array_type = value_type[1];
  static constexpr size_t kStepSize = sizeof(unit_array_type);

  constexpr ProgMemPtr() : ProgMemPtr(0) {}
  constexpr explicit ProgMemPtr(P ptr) : ptr_(ptr), loaded_temp_(false) {}

  const reference_type operator*() {
    load_temp();
    return temp_;
  }

  pointer_type operator->() {
    load_temp();
    return &temp_;
  }

  // Prefix increment
  ProgMemPtr& operator++() {
    ptr_ += kStepSize;
    return *this;
  }

  // Postfix increment
  ProgMemPtr operator++(int) {
    ProgMemPtr tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const ProgMemPtr& a, const ProgMemPtr& b) {
    return a.ptr_ == b.ptr_;
  };
  friend bool operator!=(const ProgMemPtr& a, const ProgMemPtr& b) {
    return a.ptr_ != b.ptr_;
  };

 private:
  void load_temp() {
    if (!loaded_temp_) {
      memcpy_P(&temp_, ptr_, sizeof temp_);
      loaded_temp_ = true;
    }
  }

  P ptr_;
  value_type temp_;
  bool loaded_temp_;
};

using ProgMemCharPtr = ProgMemPtr<char>;

}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_FUTURES_PROGMEM_POINTER_H_
