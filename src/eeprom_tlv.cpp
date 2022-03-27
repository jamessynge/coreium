#include "eeprom_tlv.h"

#include "eeprom_domain.h"
#include "eeprom_region.h"
#include "mcucore_platform.h"
#include "progmem_string_data.h"
#include "progmem_string_view.h"
#include "status.h"
#include "status_code.h"
#include "status_or.h"

// IDEA: consider adding something like an EepromStringView or EepromArrah<T>,
// encapsulating access to an array of chars or Ts.

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
  MCU_ASSIGN_OR_RETURN(const auto end, instance.FindEnd());
  if (!instance.IsCrcCorrect(end)) {
    return Status(StatusCode::kDataLoss, MCU_PSV("TLV CRC incorrect"));
  }
  if (!instance.IsWellFormed(end)) {
    return Status(StatusCode::kDataLoss, MCU_PSV("TLV not well formed"));
  }
  return instance;
}

void EepromTlv::ClearAndInitializeEeprom() { MCU_CHECK(false); }

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

StatusOr<EepromAddrT> EepromTlv::FindEnd() const {
  EepromAddrT end;
  eeprom_.get(kAddrOfBeyondAddr, end);
  if (kAddrOfFirstEntry <= end && end < eeprom_.length()) {
    return end;
  }
  MCU_VLOG(1) << MCU_FLASHSTR("BeyondAddr out of range: ") << end;
  return Status(StatusCode::kDataLoss, MCU_PSV("TLV end addr invalid"));
}

bool EepromTlv::IsCrcCorrect(EepromAddrT end) const {
  Crc32 computed_crc;
  for (EepromAddrT addr = kAddrOfFirstEntry; addr < end; ++addr) {
    computed_crc.appendByte(eeprom_.read(addr));
  }
  const uint32_t stored_crc = ReadCrc();
  if (computed_crc.value() == stored_crc) {
    return true;
  }
  MCU_VLOG(1) << MCU_FLASHSTR("Crc wrong: ") << stored_crc
              << "!=" << computed_crc.value();
  return false;
}

bool EepromTlv::IsWellFormed(EepromAddrT end) const {
  auto addr = kAddrOfFirstEntry;
  while (addr < end) {
    // MAYBE check validity of tag.
    auto status_or_addr = FindNext(addr);
    if (!status_or_addr.ok()) {
      return false;
    }
    addr = status_or_addr.value();
  }
  return addr == end;
}

StatusOr<EepromRegionReader> EepromTlv::FindEntry(const EepromTag& tag) const {
  MCU_ASSIGN_OR_RETURN(const auto end, FindEnd());
  EepromTag stored_tag{.domain = MCU_DOMAIN(0)};
  auto addr = kAddrOfFirstEntry;
  while (addr < end) {
    MCU_ASSIGN_OR_RETURN(const auto next_entry_addr, FindNext(addr));
    ReadTag(addr, stored_tag);
    if (tag != stored_tag) {
      addr = next_entry_addr;
      continue;
    }
    BlockLengthT entry_data_length =
        (next_entry_addr - addr) - kOffsetOfEntryData;
    eeprom_.get(addr + kOffsetOfEntryLength, entry_data_length);
    return EepromRegionReader(eeprom_, addr + kOffsetOfEntryData,
                              entry_data_length);
  }
  if (addr == end) {
    return Status(StatusCode::kNotFound);
  } else {
    return Status(StatusCode::kDataLoss, MCU_PSV("TLV end addr invalid"));
  }
}

EepromAddrT EepromTlv::ReclaimUnusedSpace() { return -1; }

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
    return Status(StatusCode::kDataLoss, MCU_PSV("entry_data_length invalid"));
  }

  return next_entry_addr;
}

uint32_t EepromTlv::ReadCrc() const {
  uint32_t crc;
  eeprom_.get(kAddrOfCrc, crc);
  return crc;
}

Status EepromTlv::StartTransaction(BlockLengthT minimum_size,
                                   EepromRegion& target_region) {
  if (transaction_is_active_) {
    return Status(StatusCode::kFailedPrecondition,
                  MCU_PSV("Write in progress"));
  }
  MCU_ASSIGN_OR_RETURN(const auto new_entry_addr, FindEnd());
  const auto new_entry_data_addr = new_entry_addr + kOffsetOfEntryData;
  const auto available =

      return Status(StatusCode::kUnimplemented);
}

Status EepromTlv::CommitTransaction(const EepromTag& tag,
                                    BlockLengthT data_length) {
  return Status(StatusCode::kUnimplemented);
}

Status EepromTlv::AbortTransaction() {
  MCU_DCHECK(transaction_is_active_);
  transaction_is_active_ = false;
  return OkStatus();
}

bool EepromTlv::Commit(EepromAddrT value_start_address,
                       EepromAddrT value_length) {
  if (pending_domain_ == RESERVED_DOMAIN) {
    MCU_VLOG(1) << MCU_FLASHSTR("Commit")
                << MCU_FLASHSTR(": No transaction is in progress!");
    return false;
  }

  return false;  // DO NOT SUBMIT TODO IMPLEMENT
}

int EepromTlv::MarkExtraBlocksUnused(const EepromTag& tag) {
  return false;  // DO NOT SUBMIT TODO IMPLEMENT
}

bool EepromTlv::MarkUnused(DataBlock& block) {
  return false;  // DO NOT SUBMIT TODO IMPLEMENT
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
