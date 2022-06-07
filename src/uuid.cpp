#include "uuid.h"

#include "hex_escape.h"
#include "logging.h"
#include "mcucore_platform.h"

namespace mcucore {
namespace {
constexpr uint_fast8_t kNumBytes = 16;

Status NotEnoughRoom() {
  return ResourceExhaustedError(
      MCU_PSV("Not enough bytes in EEPROM region for a UUID"));
}

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

Status WriteToCursorFn(EepromRegion& region, const Uuid* uuid) {
  return uuid->WriteToRegion(region);
}

}  // namespace

void Uuid::Generate() {
  for (int ndx = 0; ndx < kNumBytes; ++ndx) {
    data_[ndx] = random(256);
  }
}

void Uuid::Zero() {
  for (int ndx = 0; ndx < kNumBytes; ++ndx) {
    data_[ndx] = 0;
  }
}

Status Uuid::ReadFromRegion(EepromRegionReader& region) {
  if (region.available() < kNumBytes) {
    return NotEnoughRoom();
  }
  if (!region.ReadBytes(data_)) {
    return UnknownError(MCU_PSV("Failed to read UUID from EEPROM"));
  }
  return OkStatus();
}

Status Uuid::WriteToRegion(EepromRegion& region) const {
  if (region.available() < kNumBytes) {
    return NotEnoughRoom();
  }
  if (!region.WriteBytes(data_)) {
    return UnknownError(MCU_PSV("Failed to read UUID from EEPROM"));
  }
  return OkStatus();
}

Status Uuid::ReadFromEeprom(EepromTlv& tlv, EepromTag tag) {
  MCU_ASSIGN_OR_RETURN(auto region, tlv.FindEntry(tag));
  return ReadFromRegion(region);
}

Status Uuid::WriteToEeprom(EepromTlv& tlv, EepromTag tag) const {
  return tlv.WriteEntryToCursor(tag, kNumBytes, WriteToCursorFn, this);
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

}  // namespace mcucore
