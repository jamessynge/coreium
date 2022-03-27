#ifndef MCUCORE_SRC_EEPROM_TLV_H_
#define MCUCORE_SRC_EEPROM_TLV_H_

// EepromTlv provides support for writing and reading (aka storing and loading)
// multiple Tag-Length-Value entries in EEPROM, each with a unique tag (of type
// EepromTag).
//
// EEPROM LAYOUT
//
// 1) The prefix indicating that the EEPROM is managed by this class.
// 2) The address of the first byte beyond the entries (i.e. the place to write
//    the tag of the next entry).
// 3) The CRC-32 of all of the entries. Defaults to Crc32::kInitialValue.
// 4) Entries, each of the format:
//    a) EepromTag (i.e. domain followed by id)
//    b) Entry length, of type EepromTlv::BlockLengthT; zero is valid, in which
//       case the entry just indicates "I'm present", but has no data.
//    c) Entry data, in whatever form the writer chose.
//
// The address of the Next-Entry-To-Write and the CRC are at the start so that:
// *  We don't have to search all the entries to find them.
// *  It's an O(1) operation to figure out where to write a new entry.
// *  They don't need to be saved while a write transaction is in progress, thus
//    don't need to be restored if the write transaction is aborted.
//
// SAFETY
//
// An existing entry (i.e. the EepromRegionReader returned by
// EepromTlv::FindEntry) must not be used after any mutating method of EepromTlv
// has been called, as it may no longer represent the location of that entry or
// of a valid entry.

#include "eeprom_domain.h"
#include "eeprom_io.h"
#include "eeprom_region.h"
#include "logging.h"
#include "mcucore_platform.h"
#include "progmem_string.h"
#include "progmem_string_data.h"
#include "status_or.h"

namespace mcucore {

struct EepromTag {
  bool IsUnused() const;
  bool operator==(const EepromTag& other) const;

  // Domains are allocated to libraries or device drivers, ids are allocated
  // within a domain. Domain zero is reserved.
  EepromDomain domain;
  uint8_t id;
};

inline bool operator!=(const EepromTag& a, const EepromTag& b) {
  return !(a == b);
}

class EepromTlv {
 public:
  // We only have a modest amount of EEPROM altogether, so it doesn't make
  // sense to store individual values longer than 255 bytes.
  using BlockLengthT = uint8_t;
  static constexpr BlockLengthT kMaxBlockLength = 255;

  static StatusOr<EepromTlv> GetIfValid(EEPROMClass& eeprom);
  static StatusOr<EepromTlv> GetIfValid() { return GetIfValid(EEPROM); }

  // Search through the entries for the last one with the specified tag; most
  // of the time there will be at most one such entry, but when a replacement
  // entry or entries have been written for the same tag, and ReclaimUnusedSpace
  // has not yet been called, there may be multiple. Returns an error if the
  // EEPROM is not properly formatted, or if a block is not found with the
  // specified tag; else returns an EepromRegionReader, providing access to the
  // block.
  StatusOr<EepromRegionReader> FindEntry(const EepromTag& tag) const;

  // QUESTION: Should we keep things simple by just requiring the data to be
  // written to be provided as a byte array? If we assume that all of the data
  // is relatively small, we can just use a stack allocated byte buffer to
  // store composite data. That eliminates the idea of having an in-progress
  // transaction.

  template <typename WRITER, typename... ARGS>
  Status WriteEntryToCursor(const EepromTag& tag, BlockLengthT minimum_size,
                            WRITER writer_function, ARGS&&... writer_args) {
    EepromRegion target_region;
    MCU_RETURN_IF_ERROR(StartTransaction(minimum_size, target_region));
    if (writer_function(target_region, writer_args...)) {
      return CommitTransaction(tag, target_region.cursor());
    } else {
      return AbortTransaction();
    }
  }

  EepromAddrT ReclaimUnusedSpace();

  // Replace any contents with the standard prefix followed by zero entries.
  static void ClearAndInitializeEeprom();

 private:
  // We use instance, rather than static, methods so that testing is easier.
  explicit EepromTlv(EEPROMClass& eeprom);
  EepromTlv() : EepromTlv(EEPROM) {}

  // Returns true if the EEPROM starts with the value of Prefix().
  bool IsPrefixPresent() const;

  // Returns stored end address, if it makes sense (e.g. isn't beyond the end of
  // the EEPROM).
  StatusOr<EepromAddrT> FindEnd() const;

  // Returns true if the CRC value matches that computed from the entries.
  bool IsCrcCorrect(EepromAddrT end) const;

  // Returns true if the entries appear well formed.
  bool IsWellFormed(EepromAddrT end) const;

  // Given the address of an entry, return the address of the next entry.
  StatusOr<EepromAddrT> FindNext(EepromAddrT entry_addr) const;

  uint32_t ReadCrc() const;

  // If the EEPROM is valid, and there is sufficient space, start a write
  // transaction.
  Status StartTransaction(BlockLengthT minimum_size,
                          EepromRegion& target_region);

  Status CommitTransaction(const EepromTag& tag, BlockLengthT data_length);

  Status AbortTransaction();

  // // If there are multiple blocks with the specified tag, mark all but the
  // last
  // // one as unused. This supports first writing a replacement block, then
  // going
  // // back and
  // int MarkExtraBlocksUnused(const EepromTag& tag);

  // bool MarkUnused(DataBlock& block);

  void WriteTag(EepromAddrT entry_addr, const EepromTag& tag);
  void ReadTag(EepromAddrT entry_addr, EepromTag& tag) const;

  EEPROMClass& eeprom_;

  bool transaction_is_active_{false};
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_EEPROM_TLV_H_
