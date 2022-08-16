#include "misc/uuid.h"

#include "log/log.h"
#include "mcucore_platform.h"
#include "print/hex_escape.h"

namespace mcucore {
namespace {

// TODO(jamessynge): Move this to hex_dump or hex_escape.
size_t PrintHexBytes(Print& out, const uint8_t* bytes,
                     const uint_fast8_t num_bytes) {
  size_t result = 0;
  auto* limit = bytes + num_bytes;
  while (bytes < limit) {
    auto v = *bytes++;
    result += out.print(NibbleToAsciiHex((v >> 4) & 0xf));
    result += out.print(NibbleToAsciiHex(v & 0xf));
  }
  return result;
}

}  // namespace

void Uuid::Generate() {
  for (int ndx = 0; ndx < kNumBytes; ++ndx) {
    data_[ndx] = random(256);
  }
  // Set byte 6 to indicate this is a version 4 (random) UUID by setting
  // the high nibble to 4.
  data_[6] = (4 << 4) | (data_[6] & 0x0F);
  // Set byte 8 to indicate that this is a variant 1 UUID by setting the
  // high bit, and clearing the bit below that.
  data_[8] &= 0b00111111;
  data_[8] |= 0b10000000;
}

void Uuid::Zero() {
  for (int ndx = 0; ndx < kNumBytes; ++ndx) {
    data_[ndx] = 0;
  }
}

Status Uuid::ReadFromEeprom(EepromTlv& tlv, EepromTag tag) {
  return tlv.ReadEntry(tag, data_, kNumBytes);
}

Status Uuid::WriteToEeprom(EepromTlv& tlv, EepromTag tag) const {
  return tlv.WriteEntry(tag, data_, kNumBytes);
}

Status Uuid::ReadOrStoreEntry(EepromTlv& tlv, EepromTag tag) {
  auto status = ReadFromEeprom(tlv, tag);
  if (status.ok() || !IsNotFound(status)) {
    return status;
  }
  Generate();
  // Rather than assume we can read it back after storing it, we store it and
  // then read the value. If DCHECK is enabled, then we also make sure that the
  // value we read matches our generated value.
#ifdef MCU_ENABLE_DCHECK
  Uuid copy = *this;
  MCU_DCHECK_EQ(*this, copy) << *this << '\n' << copy;
#endif
  MCU_RETURN_IF_ERROR(WriteToEeprom(tlv, tag));
#ifdef MCU_ENABLE_DCHECK
  Zero();
#endif
  status = ReadFromEeprom(tlv, tag);
#ifdef MCU_ENABLE_DCHECK
  MCU_DCHECK_OK(status);
  MCU_DCHECK_EQ(*this, copy) << *this << '\n' << copy;
#endif
  return status;
}

size_t Uuid::printTo(Print& out) const {
  // Group 1: 4 bytes, 8 hexadecimal characters.
  size_t result = PrintHexBytes(out, data_, 4);
  result += out.print('-');
  // Group 2: 2 bytes, 4 hexadecimal characters.
  result += PrintHexBytes(out, data_ + 4, 2);
  result += out.print('-');
  // Group 3: 2 bytes, 4 hexadecimal characters.
  result += PrintHexBytes(out, data_ + 6, 2);
  result += out.print('-');
  // Group 4: 2 bytes, 4 hexadecimal characters.
  result += PrintHexBytes(out, data_ + 8, 2);
  result += out.print('-');
  // Group 5: 6 bytes, 12 hexadecimal characters.
  result += PrintHexBytes(out, data_ + 10, 6);
  return result;
}

bool operator==(const Uuid& a, const Uuid& b) {
  return memcmp(a.data_, b.data_, sizeof a.data_) == 0;
}

}  // namespace mcucore
