#ifndef MCUCORE_SRC_EEPROM_TLV_H_
#define MCUCORE_SRC_EEPROM_TLV_H_

// EepromTlv provides support for writing and reading (aka storing and loading)
// multiple Tag-Length-Value entries in EEPROM, each with a unique tag (of type
// EepromTag).
//
// Author: james.synge@gmail.com
//
// EEPROM LAYOUT
//
// 1) The prefix indicating that the EEPROM is managed by this class.
// 2) The address of the first byte beyond the entries (i.e. the place to write
//    the tag of the next entry).
// 3) The CRC-32 of all of the entries.
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
namespace test {
class EepromTlvTest;
}

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
  static constexpr EepromAddrT kFixedHeaderSize = 4 + 2 + 4;
  static constexpr EepromAddrT kEntryHeaderSize = 2 + 1;

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

  // If room for an entry with a data length of at least minimum_length is
  // available, writes an entry with the specified tag to the EEPROM. Calls
  // writer_function, passing it an EepromRegion (of at least minimum_length)
  // and writer_args; writer function should use the EepromRegion to write the
  // entry's data. If writer_function returns true, then the required updates
  // are performed to commit the entry to the EEPROM; otherwise it is not
  // considered to have been written.
  template <typename WRITER, typename... ARGS>
  Status WriteEntryToCursor(const EepromTag& tag, BlockLengthT minimum_length,
                            WRITER writer_function, ARGS&&... writer_args) {
    EepromRegion target_region;
    MCU_RETURN_IF_ERROR(StartTransaction(minimum_length, target_region));
    Status write_status = writer_function(target_region, writer_args...);
    if (write_status.ok()) {
      return CommitTransaction(tag, target_region.start_address(),
                               target_region.cursor());
    } else {
      AbortTransaction();
      return write_status;
    }
  }

  // TODO(jamessynge): Implement compaction.
  EepromAddrT ReclaimUnusedSpace();

  // Prints a bit of info, in support of googletest using OPrintStream and
  // PrintValueToStdString.
  void InsertInto(OPrintStream& strm) const;

  // Replace any contents with the standard prefix followed by zero entries.
  static void ClearAndInitializeEeprom(EEPROMClass& eeprom);
  static void ClearAndInitializeEeprom() { ClearAndInitializeEeprom(EEPROM); }

 private:
  friend class test::EepromTlvTest;

  // We use instance, rather than static, methods so that testing is easier.
  explicit EepromTlv(EEPROMClass& eeprom);
  EepromTlv() : EepromTlv(EEPROM) {}

  // Returns true if the EEPROM starts with the value of Prefix().
  bool IsPrefixPresent() const;

  // Returns stored beyond address, if it makes sense (e.g. isn't beyond the end
  // of the EEPROM).
  StatusOr<EepromAddrT> ReadBeyondAddr() const;

  // Write the beyond address.
  void WriteBeyondAddr(EepromAddrT beyond_addr);

  // Returns true if the CRC value matches that computed from the entries.
  bool IsCrcCorrect(EepromAddrT beyond_addr) const;

  // Returns true if the entries appear well formed.
  bool IsWellFormed(EepromAddrT beyond_addr) const;

  // Returns the number of bytes available for additional data.
  EepromAddrT Available() const;

  // Given the address of an entry, return the address of the next entry.
  StatusOr<EepromAddrT> FindNext(EepromAddrT entry_addr) const;

  // Store the CRC value in the EEPROM.
  void WriteCrc(uint32_t crc);

  // Read the stored CRC.
  uint32_t ReadCrc() const;

  // Compute the CRC of the entries up to, but not including beyond_addr.
  // Does not validate that the entries are well formed.
  uint32_t ComputeCrc(EepromAddrT beyond_addr) const;

  // If there is sufficient space, start a write transaction and update
  // target_region to represent the space into which the writer can write the
  // data of a new entry; target_region will have length that is at least
  // minimum_length.
  Status StartTransaction(BlockLengthT minimum_length,
                          EepromRegion& target_region);

  // Data for a new entry has been written to:
  //     [data_entry, data_entry+data_length)
  // If that range is valid, update the EEPROM to incorporate it into the TLV
  // structure.
  Status CommitTransaction(const EepromTag& tag, EepromAddrT data_addr,
                           BlockLengthT data_length);

  // Clear transaction_is_active_, to reflect that something went wrong with
  // adding a new entry, and allowing another entry to be added later.
  void AbortTransaction();

  void WriteTag(EepromAddrT entry_addr, const EepromTag& tag);
  void ReadTag(EepromAddrT entry_addr, EepromTag& tag) const;

  EEPROMClass& eeprom_;

  bool transaction_is_active_{false};
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_EEPROM_TLV_H_
