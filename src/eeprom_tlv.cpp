#include "eeprom_tlv.h"

#include "crc32.h"
#include "eeprom_region.h"
#include "eeprom_tag.h"
#include "limits.h"
#include "logging.h"
#include "mcucore_platform.h"
#include "progmem_string_data.h"
#include "progmem_string_view.h"
#include "status.h"
#include "status_code.h"
#include "status_or.h"

// IDEA: consider adding something like an EepromStringView or EepromArray<T>,
// encapsulating access to an array of chars or Ts.

// IDEA: Once well tested on host and working on device, maybe change some of
// the Status returns to MCU_DCHECKs. OR use #ifdefs to control the level of
// paranoia. OR consider adding support for something that blends MCU_VLOG_IF
// and MCU_DCHECK, such that at compile time we can decide whether the statement
// is dropped, and if retained, we can decide (at runtime?) whether it CHECKS or
// if LOGs.

// IDEA: Consider adding a VisitEntries function, for use by multiple of the
// functions below. Could be a free function in the namespace, passed in the
// EEPROMClass instance. Could be a template function, along the lines of
// WriteEntryToCursor, so that the return type could be StatusOr<T> or Status,
// depending on the needs of the called function; and of course the parameters
// of the function could be handled using a parameter pack, just as with
// WriteEntryToCursor.

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

static_assert(kAddrOfBeyondAddr == 4);
static_assert(kAddrOfCrc == 6);
static_assert(kAddrOfFirstEntry == 10);
static_assert(kAddrOfFirstEntry == EepromTlv::kFixedHeaderSize);

constexpr uint32_t kCrc32InitialValue = ~0L;

static_assert(sizeof(EepromTag) == 2);
static_assert(sizeof(EepromTlv::BlockLengthT) == 1);
constexpr EepromAddrT kOffsetOfEntryDataLength = 2;
constexpr EepromAddrT kOffsetOfEntryData = 3;

static_assert(numeric_limits<EepromTlv::BlockLengthT>::max() >=
              EepromTlv::kMaxBlockLength);

static_assert(kOffsetOfEntryData == EepromTlv::kEntryHeaderSize);

bool ValidateBeyondAddr(const EepromAddrT beyond_addr,
                        const EepromAddrT eeprom_length) {
  return kAddrOfFirstEntry <= beyond_addr && beyond_addr <= eeprom_length;
}

Status MissingPrefixError() {
  return DataLossError(MCU_PSV("TLV Prefix missing"));
}

Status WrongComputedBeyondAddr() {
  return DataLossError(MCU_PSV("Computed BeyondAddr incorrect"));
}

Status WrongCrc() { return DataLossError(MCU_PSV("TLV CRC incorrect")); }

EepromTag MakeUnusedTag() {
  return EepromTag{.domain = internal::MakeEepromDomain(0), .id = 255};
}

bool IsUnusedTag(EepromTag tag) {
  return tag.domain.value() == 0 && tag.id == 255;
}

}  // namespace

EepromTlv::EepromTlv(EEPROMClass& eeprom) : eeprom_(&eeprom) {}

StatusOr<EepromTlv> EepromTlv::GetIfValid(EEPROMClass& eeprom) {
  EepromTlv instance(eeprom);
  Status status = instance.Validate();
  if (status.ok()) {
    return instance;
  }
  if (status == MissingPrefixError()) {
    return NotFoundError(status.message());
  }
  return status;
}

StatusOr<EepromTlv> EepromTlv::ClearAndInitializeEeprom(EEPROMClass& eeprom) {
  int addr = 0;
  for (const char c : TLV_PREFIX_PSV) {
    eeprom.write(addr++, static_cast<uint8_t>(c));
  }
  EepromTlv instance(eeprom);
  MCU_DCHECK(instance.IsPrefixPresent());

#ifdef MCU_ENABLE_DCHECK
  Crc32 crc(kCrc32InitialValue);
  MCU_DCHECK_EQ(crc.value(), kCrc32InitialValue);
#endif
  instance.WriteCrc(kCrc32InitialValue);
  MCU_DCHECK_EQ(instance.ReadCrc(), kCrc32InitialValue);

  instance.WriteBeyondAddr(kAddrOfFirstEntry);
  MCU_DCHECK_OK(instance.ReadBeyondAddr().status());
  MCU_DCHECK_EQ(instance.ReadBeyondAddr().value(), kAddrOfFirstEntry);

  MCU_RETURN_IF_ERROR(instance.Validate());
  return instance;
}

StatusOr<EepromTlv> EepromTlv::Get(EEPROMClass& eeprom) {
  {
    auto status_or_instance = GetIfValid(eeprom);
    if (status_or_instance.ok()) {
      return status_or_instance;
    }
  }
  return ClearAndInitializeEeprom(eeprom);
}

EepromTlv EepromTlv::GetOrDie(EEPROMClass& eeprom) {
  auto status_or_instance = Get(eeprom);
  MCU_CHECK_OK(status_or_instance.status());
  return status_or_instance.value();
}

Status EepromTlv::Validate() const {
  if (!IsPrefixPresent()) {
    return MissingPrefixError();
  }
  MCU_ASSIGN_OR_RETURN(const auto beyond_addr, ReadBeyondAddr());
  return ValidateCrc(beyond_addr);
}

StatusOr<EepromAddrT> EepromTlv::ReclaimUnusedSpace() {
  MCU_VLOG(3) << MCU_FLASHSTR("Reclaiming deleted EepromTlv entries.");

  // Not safe to compact if ill-formed.
  MCU_RETURN_IF_ERROR(Validate());
  MCU_ASSIGN_OR_RETURN(const auto beyond_addr, ReadBeyondAddr());
  const EepromAddrT limit_addr = beyond_addr - kOffsetOfEntryData;
  MCU_VLOG_VAR(9, beyond_addr);
  MCU_VLOG_VAR(9, limit_addr);

  EepromAddrT src_addr = kAddrOfFirstEntry;
  EepromAddrT dst_addr = src_addr;
  while (src_addr <= limit_addr) {
    MCU_VLOG_VAR(9, src_addr);
    MCU_VLOG_VAR(9, dst_addr);
    MCU_DCHECK_GE(src_addr, dst_addr);
    MCU_ASSIGN_OR_RETURN(const auto next_entry_addr, FindNext(src_addr));
    MCU_VLOG_VAR(9, next_entry_addr);
    if (IsUnusedTag(ReadTag(src_addr))) {
      // Entry is unused, so we don't need to copy its bytes.
      src_addr = next_entry_addr;
      MCU_VLOG(9) << MCU_FLASHSTR("skipping unused entry");
    } else if (src_addr == dst_addr) {
      // We haven't yet found an invalid entry.
      src_addr = dst_addr = next_entry_addr;
      MCU_VLOG(9) << MCU_FLASHSTR("no unused entries so far");
    } else {
      MCU_VLOG(9) << MCU_FLASHSTR("copying used entry down by ")
                  << (src_addr - dst_addr) << MCU_FLASHSTR(" bytes");
      MCU_DCHECK_GT(src_addr, dst_addr);
      while (src_addr < next_entry_addr) {
        eeprom_->write(dst_addr++, eeprom_->read(src_addr++));
      }
    }
  }
  MCU_DCHECK_GE(src_addr, dst_addr);
  MCU_DCHECK_EQ(src_addr, beyond_addr);
  const auto new_beyond_addr = dst_addr;
  if (new_beyond_addr == beyond_addr) {
    return 0;
  }
  MCU_DCHECK_GT(beyond_addr, new_beyond_addr);

  const auto status_or_crc = ComputeCrc(new_beyond_addr);
  MCU_DCHECK_OK(status_or_crc)
      << MCU_FLASHSTR("EEPROM corrupted during compaction")      // COV_NF_LINE
      << MCU_FLASHSTR("; beyond_addr=") << beyond_addr           // COV_NF_LINE
      << MCU_FLASHSTR(", new_beyond_addr=") << new_beyond_addr;  // COV_NF_LINE
  MCU_RETURN_IF_ERROR(status_or_crc);

  WriteBeyondAddr(new_beyond_addr);
  WriteCrc(status_or_crc.value());
  auto reclaimed_space = beyond_addr - new_beyond_addr;
  MCU_VLOG(2) << MCU_FLASHSTR("Reclaimed ") << reclaimed_space
              << MCU_FLASHSTR(" EEMPROM bytes");
  return reclaimed_space;
}

StatusOr<EepromRegionReader> EepromTlv::FindEntry(const EepromTag tag) const {
  MCU_ASSIGN_OR_RETURN(const auto beyond_addr, ReadBeyondAddr());
  EepromAddrT found = 0;
  auto addr = kAddrOfFirstEntry;
  const EepromAddrT limit_addr = beyond_addr - kOffsetOfEntryData;
  while (addr <= limit_addr) {
    if (tag == ReadTag(addr)) {
      found = addr;
    }
    MCU_ASSIGN_OR_RETURN(addr, FindNext(addr));
  }

  if (addr != beyond_addr) {
    return WrongComputedBeyondAddr();
  } else if (found == 0) {
    return Status(StatusCode::kNotFound);
  }

  MCU_ASSIGN_OR_RETURN(const auto next_entry_addr, FindNext(found));
  BlockLengthT entry_data_length =
      (next_entry_addr - found) - kOffsetOfEntryData;
  eeprom_->get(found + kOffsetOfEntryDataLength, entry_data_length);
  return EepromRegionReader(*eeprom_, found + kOffsetOfEntryData,
                            entry_data_length);
}

StatusOr<EepromAddrT> EepromTlv::FindNext(EepromAddrT entry_addr) const {
  const auto entry_data_length_addr = entry_addr + kOffsetOfEntryDataLength;

  // The callers of FindNext make it (nearly?) impossible for the
  // entry_data_length_addr to be beyond the end of the EEPROM, so we just use
  // a DCHECK here, rather than returning an error if invalid.
  MCU_DCHECK_LT(entry_data_length_addr, eeprom_length())     // COV_NF_LINE
      << MCU_FLASHSTR(                                       // COV_NF_LINE
             "Invalid entry_data_length_addr, should be: ")  // COV_NF_LINE
      << entry_data_length_addr << '<' << eeprom_length();   // COV_NF_LINE

  BlockLengthT entry_data_length;
  eeprom_->get(entry_data_length_addr, entry_data_length);

  const auto entry_data_addr = entry_addr + kOffsetOfEntryData;
  const auto next_entry_addr = entry_data_addr + entry_data_length;
  if (next_entry_addr > eeprom_length()) {
    MCU_VLOG(1) << MCU_FLASHSTR("length beyond end: ") << entry_data_addr << '+'
                << entry_data_length << '>' << eeprom_length()
                << MCU_FLASHSTR("     entry_addr: ") << entry_addr;
    return Status(StatusCode::kDataLoss, MCU_PSV("data_length invalid"));
  }
  return next_entry_addr;
}

Status EepromTlv::DeleteEntry(const EepromTag tag) {
  MCU_ASSIGN_OR_RETURN(const auto beyond_addr, ReadBeyondAddr());
  MCU_ASSIGN_OR_RETURN(const bool found_other_tags,
                       DeleteEntry(tag, beyond_addr, /*not_found_ok=*/false));
  if (!found_other_tags) {
    auto status_or = ClearAndInitializeEeprom(*eeprom_);
    MCU_DCHECK_OK(status_or);  // This shouldn't fail.
    return status_or.status();
  }
  return OkStatus();
}

bool EepromTlv::IsPrefixPresent() const {
  int addr = 0;
  for (const char c : TLV_PREFIX_PSV) {
    if (c != static_cast<char>(eeprom_->read(addr++))) {
      return false;
    }
  }
  return true;
}

StatusOr<EepromAddrT> EepromTlv::ReadBeyondAddr() const {
  EepromAddrT beyond_addr;
  eeprom_->get(kAddrOfBeyondAddr, beyond_addr);
  if (ValidateBeyondAddr(beyond_addr, eeprom_length())) {
    return beyond_addr;
  }
  MCU_VLOG(1) << MCU_FLASHSTR("BeyondAddr out of range: ") << beyond_addr;
  return DataLossError(MCU_PSV("Stored BeyondAddr invalid"));
}

void EepromTlv::WriteBeyondAddr(const EepromAddrT beyond_addr) {
  eeprom_->put(kAddrOfBeyondAddr, beyond_addr);
}

EepromAddrT EepromTlv::Available() const {
  auto status_or_beyond_addr = ReadBeyondAddr();
  if (!status_or_beyond_addr.ok()) {
    return 0;
  }
  const auto new_entry_addr = status_or_beyond_addr.value();
  if (eeprom_length() - new_entry_addr <= kOffsetOfEntryData) {
    return 0;
  }
  const auto new_entry_data_addr = new_entry_addr + kOffsetOfEntryData;
  return eeprom_length() - new_entry_data_addr;
}

void EepromTlv::WriteCrc(const uint32_t crc) { eeprom_->put(kAddrOfCrc, crc); }

uint32_t EepromTlv::ReadCrc() const {
  uint32_t crc;
  eeprom_->get(kAddrOfCrc, crc);
  return crc;
}

StatusOr<uint32_t> EepromTlv::ComputeCrc(const EepromAddrT beyond_addr) const {
  Crc32 computed_crc(kCrc32InitialValue);
  auto addr = kAddrOfFirstEntry;
  const EepromAddrT limit_addr = beyond_addr - kOffsetOfEntryData;
  while (addr <= limit_addr) {
#ifdef MCU_ENABLE_DCHECK
    // Either the tag's domain is not reserved OR the tag is the unused tag.
    const auto tag = ReadTag(addr);
    MCU_DCHECK(!IsReservedDomain(tag.domain) || IsUnusedTag(tag));
#endif
    MCU_ASSIGN_OR_RETURN(const auto next_entry_addr, FindNext(addr));
    addr += kOffsetOfEntryDataLength;
    while (addr < next_entry_addr) {
      computed_crc.appendByte(eeprom_->read(addr++));
    }
  }
  if (addr == beyond_addr) {
    return computed_crc.value();
  }

  Status status = WrongComputedBeyondAddr();
  MCU_VLOG(1) << status.message() << MCU_FLASHSTR(": ") << addr
              << MCU_FLASHSTR(" != ") << beyond_addr;
  return status;
}

uint32_t EepromTlv::ComputeExtendedCrc(
    const EepromAddrT new_entry_addr, const EepromAddrT new_beyond_addr) const {
  Crc32 computed_crc(ReadCrc());
  EepromAddrT addr = new_entry_addr + kOffsetOfEntryDataLength;
  while (addr < new_beyond_addr) {
    computed_crc.appendByte(eeprom_->read(addr++));
  }
  return computed_crc.value();
}

Status EepromTlv::ValidateCrc(const EepromAddrT beyond_addr) const {
  MCU_ASSIGN_OR_RETURN(const auto computed_crc, ComputeCrc(beyond_addr));
  const uint32_t stored_crc = ReadCrc();
  if (computed_crc != stored_crc) {
    MCU_VLOG(1) << MCU_FLASHSTR("Crc wrong: ") << stored_crc
                << "!=" << computed_crc;
    return WrongCrc();
  }
  return OkStatus();
}

Status EepromTlv::StartTransaction(const EepromTag tag,
                                   const BlockLengthT minimum_length,
                                   EepromRegion& target_region,
                                   const bool reclaim_unused_space_if_needed) {
  if (IsReservedDomain(tag.domain)) {
    return InvalidArgumentError(MCU_PSV("Domain is reserved"));
  }
  if (minimum_length > kMaxBlockLength) {
    return InvalidArgumentError(MCU_PSV("minimum_length too large"));
  }
  MCU_RETURN_IF_ERROR(ValidateNoTransactionIsActive());
  MCU_ASSIGN_OR_RETURN(const auto new_entry_addr, ReadBeyondAddr());
  MCU_DCHECK_OK(ValidateCrc(new_entry_addr));
  const auto new_entry_data_addr = new_entry_addr + kOffsetOfEntryData;
  const auto available = eeprom_length() - new_entry_data_addr;
  if (available >= minimum_length) {
    EepromAddrT length;
    if (available > kMaxBlockLength) {
      length = kMaxBlockLength;
    } else {
      length = available;
    }
    target_region = EepromRegion(*eeprom_, new_entry_data_addr, length);
    transaction_is_active_ = true;
    return OkStatus();
  } else if (reclaim_unused_space_if_needed) {
    // There isn't enough room, but we're allowed to reclaim unused space,
    // so let's try.
    MCU_ASSIGN_OR_RETURN(const auto reclaimed_space, ReclaimUnusedSpace());
    if (available + reclaimed_space >= minimum_length) {
      // Try again, but don't try reclaiming space again.
      return StartTransaction(tag, minimum_length, target_region,
                              /*reclaim_unused_space_if_needed=*/false);
    }
  }
  return Status(StatusCode::kResourceExhausted);
}

Status EepromTlv::ValidateNoTransactionIsActive() const {
  if (transaction_is_active_) {
    return Status(StatusCode::kFailedPrecondition,
                  MCU_PSV("Write in progress"));
  }
  return OkStatus();
}

Status EepromTlv::CommitTransaction(const EepromTag tag,
                                    const EepromAddrT data_addr,
                                    const BlockLengthT data_length) {
  if (!transaction_is_active_) {
    return InternalError(MCU_PSV("Write NOT in progress"));  // COV_NF_LINE
  }
  transaction_is_active_ = false;
  if (data_length > kMaxBlockLength) {
    return InternalError(MCU_PSV("data_length too large"));
  }
  MCU_ASSIGN_OR_RETURN(const auto new_entry_addr, ReadBeyondAddr());
  const auto new_entry_data_addr = new_entry_addr + kOffsetOfEntryData;
  if (new_entry_data_addr != data_addr) {
    return InternalError(MCU_PSV("Commit wrong data_addr"));
  }
  MCU_DCHECK_OK(ValidateCrc(new_entry_addr));

  const auto new_beyond_addr = new_entry_data_addr + data_length;
  if (new_beyond_addr > eeprom_length()) {
    MCU_VLOG(1) << MCU_FLASHSTR("length beyond end: ")        // COV_NF_LINE
                << new_entry_data_addr << '+' << data_length  // COV_NF_LINE
                << '>' << eeprom_length();                    // COV_NF_LINE
    return DataLossError(MCU_PSV("data_length invalid"));     // COV_NF_LINE
  }

  // Write the Tag and Length of the entry.
  WriteTag(new_entry_addr, tag);
  WriteEntryDataLength(new_entry_addr, data_length);

  // We shouldn't yet have touched anything before the new entry, so the
  // contents before new_entry_addr should still be valid.
  MCU_DCHECK_OK(Validate());

  // Compute the CRC incrementally, i.e. be reading the old value, then adding
  // the appropriate bytes of the new entry.
  const uint32_t extended_crc =
      ComputeExtendedCrc(new_entry_addr, new_beyond_addr);

#ifdef MCU_ENABLE_DCHECK
  // Validate that the extended CRC matches what we'll get if we compute the
  // value from all of the entries.
  auto status_or_full_crc = ComputeCrc(new_beyond_addr);
  MCU_DCHECK_OK(status_or_full_crc);
  MCU_DCHECK_EQ(extended_crc, status_or_full_crc.value());
#endif

  // So far we've not changed anything prior to new_entry_addr. Now we need to
  // update the stored CRC and the stored beyond addr.
  WriteCrc(extended_crc);
  WriteBeyondAddr(new_beyond_addr);

  // Validate should now
  MCU_DCHECK_OK(Validate());

  // 'Remove' any other entries with the same tag that appear before the new
  // entry.
  MCU_RETURN_IF_ERROR(DeleteEntry(tag, new_entry_addr, /*not_found_ok=*/true));

  // And a last step of paranoia.
  MCU_DCHECK_OK(Validate());

  return OkStatus();
}

void EepromTlv::AbortTransaction() {
  MCU_DCHECK(transaction_is_active_);
  transaction_is_active_ = false;
}

EepromTag EepromTlv::ReadTag(EepromAddrT entry_addr) const {
  // This is one of only two places where we call MakeEepromDomain directly.
  EepromTag tag{.domain = internal::MakeEepromDomain(eeprom_->read(entry_addr)),
                .id = eeprom_->read(entry_addr + 1)};
  return tag;
}

void EepromTlv::WriteTag(EepromAddrT entry_addr, const EepromTag tag) {
  eeprom_->write(entry_addr, tag.domain.value());
  eeprom_->write(entry_addr + 1, tag.id);
}

EepromTlv::BlockLengthT EepromTlv::ReadEntryDataLength(
    EepromAddrT entry_addr) const {
  BlockLengthT data_length;
  eeprom_->get(entry_addr + kOffsetOfEntryDataLength, data_length);
  return data_length;
}

void EepromTlv::WriteEntryDataLength(EepromAddrT entry_addr,
                                     BlockLengthT data_length) {
  const auto entry_length_addr = entry_addr + kOffsetOfEntryDataLength;
  eeprom_->put(entry_length_addr, data_length);
}

StatusOr<bool> EepromTlv::DeleteEntry(const EepromTag tag,
                                      const EepromAddrT beyond_addr,
                                      const bool not_found_ok) {
  MCU_RETURN_IF_ERROR(ValidateNoTransactionIsActive());
  bool found_other_tags = false;
  bool found_tag = not_found_ok;
  auto addr = kAddrOfFirstEntry;
  const EepromAddrT limit_addr = beyond_addr - kOffsetOfEntryData;
  while (addr <= limit_addr) {
    const auto stored_tag = ReadTag(addr);
    if (tag == stored_tag) {
      found_tag = true;
      WriteTag(addr, MakeUnusedTag());
    } else if (!IsUnusedTag(stored_tag)) {
      found_other_tags = true;
    }
    MCU_ASSIGN_OR_RETURN(addr, FindNext(addr));
  }
  if (addr != beyond_addr) {
    return Status(StatusCode::kDataLoss, MCU_PSV("TLV addr != beyond_addr"));
  } else if (!found_tag) {
    return Status(StatusCode::kNotFound);
  } else {
    return found_other_tags;
  }
}

// Debugging support. This doesn't use methods which print error messages so
// that it can be used to print error messages without causing recursion.
void EepromTlv::InsertInto(OPrintStream& strm) const {
  strm << MCU_FLASHSTR("{Prefix:");
  if (IsPrefixPresent()) {
    strm << MCU_FLASHSTR("OK");
  } else {
    strm << MCU_FLASHSTR("Missing");
  }

  EepromAddrT beyond_addr;
  eeprom_->get(kAddrOfBeyondAddr, beyond_addr);
  strm << MCU_FLASHSTR(", ") << MCU_FLASHSTR("Beyond=") << beyond_addr;
  if (!ValidateBeyondAddr(beyond_addr, eeprom_length())) {
    strm << MCU_FLASHSTR(" (Invalid)");
  } else if (beyond_addr == kAddrOfFirstEntry) {
    strm << MCU_FLASHSTR(" (Empty)");
  } else if (beyond_addr > (eeprom_length() - kOffsetOfEntryData)) {
    strm << MCU_FLASHSTR(" (Full)");
  }

  strm << MCU_FLASHSTR(", ") << MCU_FLASHSTR("Crc=") << ReadCrc();

  // IDEA: Compute CRC as we go, print out the value if it doesn't match the
  // stored CRC.
  auto addr = kAddrOfFirstEntry;
  const EepromAddrT limit_addr = beyond_addr - kOffsetOfEntryData;
  while (addr <= limit_addr) {
    strm << MCU_FLASHSTR(",\n   ");
    const auto tag = ReadTag(addr);
    strm << MCU_FLASHSTR("Entry@") << addr << MCU_FLASHSTR("{tag=") << tag;
    if (IsUnusedTag(tag)) {
      strm << MCU_FLASHSTR(" (Unused)");
    } else if (IsReservedDomain(tag.domain)) {
      strm << MCU_FLASHSTR(" (Reserved)");
    }
    const auto data_length = ReadEntryDataLength(addr);
    const auto next_entry_addr = addr + kOffsetOfEntryData + data_length;
    strm << MCU_FLASHSTR(", length=") << data_length << MCU_FLASHSTR(", next=")
         << next_entry_addr << '}';
    addr = next_entry_addr;
  }

  if (addr != kAddrOfFirstEntry) {
    strm << '\n';
  }
  if (addr != beyond_addr) {
    strm << MCU_FLASHSTR("MISALIGNED");
  }

  strm << '}';
}

}  // namespace mcucore
