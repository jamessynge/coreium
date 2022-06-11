#ifndef MCUCORE_SRC_PROGMEM_POINTER_H_
#define MCUCORE_SRC_PROGMEM_POINTER_H_

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
class ProgmemPtr {
 public:
  using value_type = T;
  using reference_type = value_type&;
  using pointer_type = value_type*;
  using unit_array_type = value_type[1];
  static constexpr size_t kStepSize = sizeof(unit_array_type);

  constexpr ProgmemPtr() : ProgmemPtr(0) {}
  constexpr explicit ProgmemPtr(P ptr) : ptr_(ptr), loaded_temp_(false) {}

  const reference_type operator*() {
    load_temp();
    return temp_;
  }

  pointer_type operator->() {
    load_temp();
    return &temp_;
  }

  // Prefix increment
  ProgmemPtr& operator++() {
    ptr_ += kStepSize;
    loaded_temp_ = false;
    return *this;
  }

  // Postfix increment
  ProgmemPtr operator++(int) {
    // Make a copy to be returned.
    ProgmemPtr tmp(ptr_);
    // Then modify this instance.
    ptr_ += kStepSize;
    loaded_temp_ = false;
    return tmp;
  }

  friend bool operator==(const ProgmemPtr& a, const ProgmemPtr& b) {
    return a.ptr_ == b.ptr_;
  }

  friend bool operator!=(const ProgmemPtr& a, const ProgmemPtr& b) {
    return a.ptr_ != b.ptr_;
  }

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

using ProgmemCharPtr = ProgmemPtr<char>;

}  // namespace mcucore

#endif  // MCUCORE_SRC_PROGMEM_POINTER_H_
