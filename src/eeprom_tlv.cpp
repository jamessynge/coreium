#include "eeprom_tlv.h"

#include "crc32.h"
#include "eeprom_domain.h"
#include "eeprom_region.h"
#include "logging.h"
#include "mcucore_platform.h"
#include "progmem_string_data.h"
#include "progmem_string_view.h"
#include "status.h"
#include "status_code.h"
#include "status_or.h"

// IDEA: consider adding something like an EepromStringView or EepromArrah<T>,
// encapsulating access to an array of chars or Ts.

// IDEA: Once well tested on host and working on device, maybe change some of
// the Status returns to MCU_DCHECKs. OR use #ifdefs to control the level of
// paranoia. OR consider adding support for something that blends MCU_VLOG_IF
// and MCU_DCHECK, such that at compile time we can decide whether the statement
// is dropped, and if retained, we can decide (at runtime?) whether it CHECKS or
// if LOGs.

// We reserve domain 0 and domain 255 because bytes in the the EEPROM are set
// to one of those values when cleared/erased, but it isn't clear which one, and
// historically different EEPROM designs have used one or the other of those two
// values. Avoiding them allows us to double check that stored domains are
// valid. For Microchip AVR microcontrollers, it seems most likely that 255
// 0xFF) is the cleared value.
MCU_DEFINE_DOMAIN(0);
MCU_DEFINE_DOMAIN(255);

#define TLV_RAW_PREFIX "Tlv!"
#define TLV_PREFIX_PSV MCU_PSV(TLV_RAW_PREFIX)
#define TLV_PREFIX_SIZE (decltype(MCU_PSD(TLV_RAW_PREFIX))::size())

namespace mcucore {
namespace {
// TODO(jamessynge): Determine whether these (and other) constexpr values take
// storage (RAM or Flash) when built with avr-gcc.

constexpr EepromAddrT kAddrOfBeyondAddr = TLV_PREFIX_SIZE;
constexpr EepromAddrT kAddrOfCrc = kAddrOfBeyondAddr + sizeof(EepromAddrT);
constexpr EepromAddrT kAddrOfFirstEntry = kAddrOfCrc + sizeof(uint32_t);

constexpr uint32_t kCrc32InitialValue = ~0L;

static_assert(sizeof(EepromTag) == 2);
static_assert(sizeof(EepromTlv::BlockLengthT) == 1);
constexpr EepromAddrT kOffsetOfEntryLength = 2;
constexpr EepromAddrT kOffsetOfEntryData = 3;

}  // namespace

bool EepromTag::IsUnused() const {
  return (domain == MCU_DOMAIN(0) && id == 255) ||
         (domain == MCU_DOMAIN(255) && id == 0);
}

bool EepromTag::operator==(const EepromTag& other) const {
  return domain == other.domain && id == other.id;
}

StatusOr<EepromTlv> EepromTlv::GetIfValid(EEPROMClass& eeprom) {
  EepromTlv instance(eeprom);
  if (!instance.IsPrefixPresent()) {
    return Status(StatusCode::kNotFound, MCU_PSV("TLV Prefix missing"));
  }
  MCU_ASSIGN_OR_RETURN(const auto beyond_addr, instance.ReadBeyondAddr());
  if (!instance.IsCrcCorrect(beyond_addr)) {
    return Status(StatusCode::kDataLoss, MCU_PSV("TLV CRC incorrect"));
  }
  if (!instance.IsWellFormed(beyond_addr)) {
    return Status(StatusCode::kDataLoss, MCU_PSV("TLV not well formed"));
  }
  return instance;
}

void EepromTlv::ClearAndInitializeEeprom(EEPROMClass& eeprom) {
  int addr = 0;
  for (const char c : TLV_PREFIX_PSV) {
    eeprom.write(addr++, static_cast<uint8_t>(c));
  }
  Crc32 crc(kCrc32InitialValue);
  EepromTlv instance(eeprom);
  instance.WriteCrc(crc.value());
  instance.WriteBeyondAddr(kAddrOfFirstEntry);
}

EepromTlv::EepromTlv(EEPROMClass& eeprom) : eeprom_(eeprom) {}

bool EepromTlv::IsPrefixPresent() const {
  int addr = 0;
  for (const char c : TLV_PREFIX_PSV) {
    if (c != static_cast<char>(eeprom_.read(addr++))) {
      return false;
    }
  }
  return true;
}

StatusOr<EepromAddrT> EepromTlv::ReadBeyondAddr() const {
  EepromAddrT beyond_addr;
  eeprom_.get(kAddrOfBeyondAddr, beyond_addr);
  if (kAddrOfFirstEntry <= beyond_addr && beyond_addr < eeprom_.length()) {
    return beyond_addr;
  }
  MCU_VLOG(1) << MCU_FLASHSTR("BeyondAddr out of range: ") << beyond_addr;
  return Status(StatusCode::kDataLoss, MCU_PSV("TLV beyond_addr invalid"));
}

void EepromTlv::WriteBeyondAddr(const EepromAddrT beyond_addr) {
  eeprom_.put(kAddrOfBeyondAddr, beyond_addr);
}

bool EepromTlv::IsCrcCorrect(const EepromAddrT beyond_addr) const {
  const auto computed_crc = ComputeCrc(beyond_addr);
  const uint32_t stored_crc = ReadCrc();
  if (computed_crc == stored_crc) {
    return true;
  }
  MCU_VLOG(1) << MCU_FLASHSTR("Crc wrong: ") << stored_crc
              << "!=" << computed_crc;
  return false;
}

bool EepromTlv::IsWellFormed(const EepromAddrT beyond_addr) const {
  auto addr = kAddrOfFirstEntry;
  while (addr < beyond_addr) {
    // MAYBE check validity of tag.
    auto status_or_addr = FindNext(addr);
    if (!status_or_addr.ok()) {
      return false;
    }
    addr = status_or_addr.value();
  }
  return addr == beyond_addr;
}

StatusOr<EepromRegionReader> EepromTlv::FindEntry(const EepromTag& tag) const {
  MCU_ASSIGN_OR_RETURN(const auto beyond_addr, ReadBeyondAddr());
  EepromTag stored_tag{.domain = MCU_DOMAIN(0)};
  EepromAddrT found = 0;
  auto addr = kAddrOfFirstEntry;
  while (addr < beyond_addr) {
    ReadTag(addr, stored_tag);
    if (tag == stored_tag) {
      found = addr;
    }
    MCU_ASSIGN_OR_RETURN(addr, FindNext(addr));
  }

  if (addr != beyond_addr) {
    return Status(StatusCode::kDataLoss, MCU_PSV("TLV beyond_addr invalid"));
  } else if (found == 0) {
    return Status(StatusCode::kNotFound);
  }

  MCU_ASSIGN_OR_RETURN(const auto next_entry_addr, FindNext(found));
  BlockLengthT entry_data_length =
      (next_entry_addr - found) - kOffsetOfEntryData;
  eeprom_.get(found + kOffsetOfEntryLength, entry_data_length);
  return EepromRegionReader(eeprom_, found + kOffsetOfEntryData,
                            entry_data_length);
}

EepromAddrT EepromTlv::ReclaimUnusedSpace() {
  MCU_CHECK(false) << MCU_FLASHSTR("ReclaimUnusedSpace is unimplemented");
  return -1;
}

StatusOr<EepromAddrT> EepromTlv::FindNext(EepromAddrT entry_addr) const {
  const auto entry_data_addr = entry_addr + kOffsetOfEntryData;
  if ((entry_data_addr + sizeof entry_data_addr) > eeprom_.length()) {
    MCU_VLOG(1) << MCU_FLASHSTR("data addr out of range: ") << entry_data_addr
                << '+' << sizeof entry_data_addr << '>' << eeprom_.length();
    return Status(StatusCode::kDataLoss, MCU_PSV("entry_data_addr invalid"));
  }
  EepromAddrT entry_data_length;
  eeprom_.get(entry_addr + kOffsetOfEntryLength, entry_data_length);

  const auto next_entry_addr = entry_data_addr + entry_data_length;
  if (next_entry_addr > eeprom_.length()) {
    MCU_VLOG(1) << MCU_FLASHSTR("length beyond end: ") << entry_data_addr << '+'
                << entry_data_length << '>' << eeprom_.length();
    return Status(StatusCode::kDataLoss, MCU_PSV("data_length invalid"));
  }

  return next_entry_addr;
}

uint32_t EepromTlv::ReadCrc() const {
  uint32_t crc;
  eeprom_.get(kAddrOfCrc, crc);
  return crc;
}

uint32_t EepromTlv::ComputeCrc(const EepromAddrT beyond_addr) const {
  Crc32 computed_crc(kCrc32InitialValue);
  for (EepromAddrT addr = kAddrOfFirstEntry; addr < beyond_addr; ++addr) {
    computed_crc.appendByte(eeprom_.read(addr));
  }
  return computed_crc.value();
}

void EepromTlv::WriteCrc(const uint32_t crc) { eeprom_.write(kAddrOfCrc, crc); }

Status EepromTlv::StartTransaction(const BlockLengthT minimum_length,
                                   EepromRegion& target_region) {
  if (transaction_is_active_) {
    return Status(StatusCode::kFailedPrecondition,
                  MCU_PSV("Write in progress"));
  }
  MCU_ASSIGN_OR_RETURN(const auto new_entry_addr, ReadBeyondAddr());
  const auto new_entry_data_addr = new_entry_addr + kOffsetOfEntryData;
  const auto available = eeprom_.length() - new_entry_data_addr;
  if (available < minimum_length) {
    return Status(StatusCode::kResourceExhausted);
  }
  EepromAddrT length;
  if (available > kMaxBlockLength) {
    length = kMaxBlockLength;
  } else {
    length = available;
  }
  target_region = EepromRegion(eeprom_, new_entry_data_addr, length);
  transaction_is_active_ = true;
  return OkStatus();
}

Status EepromTlv::CommitTransaction(const EepromTag& tag,
                                    const EepromAddrT data_addr,
                                    const BlockLengthT data_length) {
  if (!transaction_is_active_) {
    return Status(StatusCode::kInternal, MCU_PSV("Write NOT in progress"));
  }
  transaction_is_active_ = false;
  MCU_ASSIGN_OR_RETURN(const auto new_entry_addr, ReadBeyondAddr());
  const auto new_entry_data_addr = new_entry_addr + kOffsetOfEntryData;
  if (new_entry_data_addr != data_addr) {
    return Status(StatusCode::kInternal, MCU_PSV("Commit wrong data_addr"));
  }
  const auto new_beyond_addr = new_entry_data_addr + data_length;
  if (new_beyond_addr > eeprom_.length()) {
    MCU_VLOG(1) << MCU_FLASHSTR("length beyond end: ") << new_entry_data_addr
                << '+' << data_length << '>' << eeprom_.length();
    return Status(StatusCode::kDataLoss, MCU_PSV("data_length invalid"));
  }

  // Write the Tag and Length of the entry.
  WriteTag(new_entry_addr, tag);
  eeprom_.put(new_entry_addr + kOffsetOfEntryLength, data_length);

  // So far we've not changed anything prior to new_entry_addr. Now we need to
  // compute the new CRC. For now doing so by starting from the current stored
  // value and appending just the bytes after the last entry, though we also
  // have a DCHECK to confirm that process has produced the same values as
  // computing the value from all bytes.

  MCU_DCHECK(IsCrcCorrect(new_entry_addr));
  Crc32 crc32(ReadCrc());
  for (EepromAddrT addr = new_entry_data_addr; addr < new_beyond_addr; ++addr) {
    crc32.appendByte(eeprom_.read(addr));
  }
  MCU_DCHECK_EQ(ComputeCrc(new_beyond_addr), crc32.value());

  // We still haven't changed anything prior to new_entry_addr, but now we
  // finally need to update the stored CRC and the stored beyond addr.

  WriteCrc(crc32.value());
  WriteBeyondAddr(new_beyond_addr);
  return OkStatus();
}

void EepromTlv::AbortTransaction() {
  MCU_DCHECK(transaction_is_active_);
  transaction_is_active_ = false;
}

void EepromTlv::WriteTag(EepromAddrT entry_addr, const EepromTag& tag) {
  eeprom_.write(entry_addr, tag.domain.value());
  eeprom_.write(entry_addr + 1, tag.id);
}

void EepromTlv::ReadTag(EepromAddrT entry_addr, EepromTag& tag) const {
  // This is one of only two places where we call MakeEepromDomain directly.
  tag.domain = internal::MakeEepromDomain(eeprom_.read(entry_addr));
  tag.id = eeprom_.read(entry_addr + 1);
}

}  // namespace mcucore
