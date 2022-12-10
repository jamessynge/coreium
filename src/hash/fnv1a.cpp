#include "hash/fnv1a.h"

#include "log/log.h"

#define FNV1A_PRIME 0x01000193  // 2^24 + 2^8 + 0x93
#define FNV1A_INITIAL_VALUE 2166136261
#define FNV1A_BIT_SHIFT_INSTEAD_OF_MULTIPLY  // Measure whether this has any
                                             // benefit on the 8-bit AVR
                                             // processors.

namespace mcucore {

Fnv1a::Fnv1a() : value_(FNV1A_INITIAL_VALUE) {}

void Fnv1a::appendByte(uint8_t v) {
  MCU_DCHECK_NE(value_, 0);

  // XOR the low order byte with v.

#ifndef NDEBUG  // Rarely needed.
  MCU_DCHECK_EQ((value_ ^ v), (value_ ^ static_cast<uint32_t>(v)));
#endif
  uint32_t temp_value = value_ ^ v;

  // Multiply by the 32 bit FNV magic prime, mod 2^32. The mod 2^32 is required
  // by the C++ (and C?) standard. See:
  //
  //   https://en.cppreference.com/w/cpp/language/operator_arithmetic#Overflows
  //
  // Here this means that while the intermediate result is (logically) a
  // uint64_t, it is converted to a uint32_t by retaining only the low 32 bits
  // of that intermediate result, and thus loses the information in the high 32
  // bits.

#ifdef FNV1A_BIT_SHIFT_INSTEAD_OF_MULTIPLY
  // This could probably be further optimized for an 8-bit processor.
  temp_value += ((temp_value << 1) + (temp_value << 4) + (temp_value << 7) +
                 (temp_value << 8) + (temp_value << 24));
#else   // !FNV1A_BIT_SHIFT_INSTEAD_OF_MULTIPLY
  temp_value *= FNV1A_PRIME;
#endif  // FNV1A_BIT_SHIFT_INSTEAD_OF_MULTIPLY

  // Hack to avoid hashing to zero.
  if (temp_value == 0) {
    temp_value = FNV1A_INITIAL_VALUE ^ v;
  }

  MCU_VLOG(MCUCORE_FNV1A_APPEND_BYTE_LOG_LEVEL)
      << MCU_PSD("Fnv1a::appendByte(") << (v + 0) << MCU_PSD(") old value_=")
      << BaseHex << value_ << MCU_PSD(", new value_=") << BaseHex << temp_value;

  value_ = temp_value;
}

}  // namespace mcucore
