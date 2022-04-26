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
// 3) The CRC-32 of all of the lengths and data; the EepromTags are not included
//    in the CRC because that allows us to invalidate an entry by changing the
//    tag, without the need to recompute the CRC.
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

#include "eeprom_region.h"
#include "eeprom_tag.h"
#include "logging.h"
#include "mcucore_platform.h"
#include "progmem_string_data.h"
#include "status_or.h"

namespace mcucore {
namespace test {
class EepromTlvTest;
}

class EepromTlv {
 public:
  // We only have a modest amount of EEPROM altogether, so it doesn't make
  // sense to store individual values longer than 255 bytes.
  using BlockLengthT = uint8_t;
  static constexpr BlockLengthT kMaxBlockLength = 255;
  static constexpr EepromAddrT kFixedHeaderSize = 4 + 4 + 2;
  static constexpr EepromAddrT kEntryHeaderSize = 2 + 1;

  // Gets an instance of EepromTlv, if the EEPROM contains data in the expected
  // format.
  static StatusOr<EepromTlv> GetIfValid(EEPROMClass& eeprom);
  static StatusOr<EepromTlv> GetIfValid() { return GetIfValid(EEPROM); }

  // Format the EEPROM as an empty EepromTlv, and return an instance. Fails only
  // if the EEPROM can't be initialized (e.g. doesn't hold the expected values
  // after writing).
  static StatusOr<EepromTlv> ClearAndInitializeEeprom(EEPROMClass& eeprom);
  static StatusOr<EepromTlv> ClearAndInitializeEeprom() {
    return ClearAndInitializeEeprom(EEPROM);
  }

  // Get an EepromTlv instance for the EEPROM. If already valid, does not modify
  // the EEPROM; otherwise, uses ClearAndInitializeEeprom to make the EEPROM
  // represent an empty EepromTlv instance.
  static StatusOr<EepromTlv> Get(EEPROMClass& eeprom);
  static StatusOr<EepromTlv> Get() { return Get(EEPROM); }

  // Returns a valid EepromTlv instance, clearing the EEPROM if necessary. Fails
  // only if a non-initialized EEPROM can't be initialized.
  static EepromTlv GetOrDie(EEPROMClass& eeprom);
  static EepromTlv GetOrDie() { return GetOrDie(EEPROM); }

  // Returns an OK Status if the instance is valid, else an appropriate error
  // Status.
  Status Validate() const;

  // Remove any deleted entries by compacting the valid entries so there is no
  // unused space prior to the end of the valid entries.
  StatusOr<EepromAddrT> ReclaimUnusedSpace();

  // Returns an EepromRegionReader for reading the data of the most recently
  // written entry with the specified tag. Returns an error if the EEPROM is not
  // properly formatted, or if a block is not found with the specified tag.
  StatusOr<EepromRegionReader> FindEntry(EepromTag tag) const;

  // Delete the entry (or entries) with the specified tag. Returns an error if
  // the EEPROM is not properly formatted, or if not entry is found with the
  // specified tag.
  //
  // Due to the nature of EEPROM, instead of "removing" the entry so that the
  // data isn't in the EEPROM, the implementation simply modifies any existing
  // tags that match so that they will no longer match; i.e. such entries will
  // be marked as unused space.
  Status DeleteEntry(EepromTag tag);

  // QUESTION: Should we keep things simple by just requiring the data to be
  // written to be provided as a byte array? If we assume that all of the data
  // is relatively small, we can just use a stack allocated byte buffer to
  // store composite data. That eliminates the idea of having an in-progress
  // transaction.

  // Adds a new entry to the EEPROM, if there is at least minimum_length is
  // available. Does so by calling writer_function with an EepromRegion instance
  // configured to write to the data portion of a new entry. If the function
  // returns an OK Status, then the entry's tag and length (equal to the
  // EepromRegion's cursor upon the return of writer_function) are written to
  // the EEPROM, and the CRC and Beyond Addr are also updated. If an error
  // occurs at any point, an error Status is returned and the transaction is
  // aborted.
  template <typename WRITER, typename... ARGS>
  Status WriteEntryToCursor(EepromTag tag, BlockLengthT minimum_length,
                            WRITER writer_function, ARGS&&... writer_args) {
    EepromRegion target_region;
    MCU_RETURN_IF_ERROR(StartTransaction(tag, minimum_length, target_region));
    Status status = writer_function(target_region, writer_args...);
    if (!status.ok()) {
      AbortTransaction();
      return status;
    }
    return CommitTransaction(tag, target_region.start_address(),
                             target_region.cursor());
  }

  // Prints a bit of info, in support of googletest using OPrintStream and
  // PrintValueToStdString.
  void InsertInto(OPrintStream& strm) const;

  // Returns the number of bytes available for additional data. Public to make
  // testing easier.
  EepromAddrT Available() const;

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

  // Given the address of an entry, return the address of the next entry.
  StatusOr<EepromAddrT> FindNext(EepromAddrT entry_addr) const;

  // Store the CRC value in the EEPROM.
  void WriteCrc(uint32_t crc);

  // Read the stored CRC.
  uint32_t ReadCrc() const;

  // Compute the CRC of the entries up to, but not including beyond_addr, where
  // beyond addr must be the address of:
  // A) the first entry (i.e. when there are no entries yet);
  // B) the address just beyond the last entry.
  // This has the effect of validating the structure of the entries.
  StatusOr<uint32_t> ComputeCrc(EepromAddrT beyond_addr) const;

  // Given the address of a new entry, and the address immediately beyond that
  // entry, compute the CRC of all entries by appending just the relevant bytes
  // of the new entry.
  uint32_t ComputeExtendedCrc(EepromAddrT new_entry_addr,
                              EepromAddrT beyond_addr) const;

  // Returns true if the CRC value matches that computed from the entries.
  Status ValidateCrc(EepromAddrT beyond_addr) const;

  // If there is sufficient space, start a write transaction and update
  // target_region to represent the space into which the writer can write the
  // data of a new entry; target_region will have length that is at least
  // minimum_length.
  Status StartTransaction(EepromTag tag, BlockLengthT minimum_length,
                          EepromRegion& target_region);

  // Data for a new entry has been written to:
  //     [data_entry, data_entry+data_length)
  // If that range is valid, update the EEPROM to incorporate it into the TLV
  // structure.
  Status CommitTransaction(EepromTag tag, EepromAddrT data_addr,
                           BlockLengthT data_length);

  // Clear transaction_is_active_, to reflect that something went wrong with
  // adding a new entry, and allowing another entry to be added later.
  void AbortTransaction();

  EepromTag ReadTag(EepromAddrT entry_addr) const;
  void WriteTag(EepromAddrT entry_addr, EepromTag tag);

  BlockLengthT ReadEntryDataLength(EepromAddrT entry_addr) const;
  void WriteEntryDataLength(EepromAddrT entry_addr, BlockLengthT data_length);

  // Removes entries that are before beyond_addr. If any entries are removed, or
  // if not_found_ok is true, then the value returned is true if any non-removed
  // entries are found. If no entries are removed AND not_found_ok is false,
  // then a NotFound error is returned. Other errors may be returned if the
  // EEPROM is not well formed.
  StatusOr<bool> DeleteEntry(EepromTag tag, EepromAddrT beyond_addr,
                             bool not_found_ok);

  Status ValidateNoTransactionIsActive() const;

  EepromAddrT eeprom_length() const {
    return const_cast<EEPROMClass&>(*eeprom_).length();
  }

  // We use a ptr rather than a ref here to allow copy and move assignment to
  // compile.
  EEPROMClass* eeprom_;

  bool transaction_is_active_{false};
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_EEPROM_TLV_H_
