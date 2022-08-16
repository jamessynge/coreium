#ifndef MCUCORE_SRC_HASH_FNV1A_H_
#define MCUCORE_SRC_HASH_FNV1A_H_

// Class for computing a 32-bit hash value using FNV-1a. FNV (Fowler/Noll/Vo) is
// a fast, non-cryptographic hash algorithm.
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
//
// algorithm fnv-1a is
//    hash := FNV_offset_basis
//    for each byte_of_data to be hashed do
//        hash := hash XOR byte_of_data
//        hash := hash × FNV_prime
//    return hash
//
// Author: james.synge@gmail.com (well, of this adaptation of the idea).

#include <stdint.h>

namespace mcucore {

class Fnv1a {
 public:
  Fnv1a();

  // Add the next byte of the sequence on which we're computing a hash.
  void appendByte(uint8_t v);

  // The current value of the hash.
  uint32_t value() const { return value_; }

 private:
  uint32_t value_;
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_HASH_FNV1A_H_
