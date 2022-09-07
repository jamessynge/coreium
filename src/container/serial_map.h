#ifndef MCUCORE_SRC_CONTAINER_SERIAL_MAP_H_
#define MCUCORE_SRC_CONTAINER_SERIAL_MAP_H_

// SerialMap<KEY, SIZE> is a map from KEY to variable length values stored as a
// length (uint8_t) and an array of bytes of that length. The caller is required
// to know what the "real" type of the value bytes are (e.g. by only storing a
// certain type of value for a particular key value). The class is called
// SerialMap because the entries are stored in a single array of bytes, i.e. are
// stored as if serialized.
//
// This provides features similar to those of EepromTlv, but for the case where
// we have a single, fixed size block (of size SIZE bytes) of RAM available in
// which to store the map. The aim is to support storing HTTP parameter and
// header values when decoding an HTTP request, where the handlers for different
// paths may have different expectations regarding the set of parameters that
// are supported or required, and where those values may vary in size.
//
// Author: james.synge@gmail.com

#include <string.h>  // For memcpy, etc.

#include "log/log.h"
#include "mcucore_platform.h"
#include "semistd/limits.h"
#include "semistd/type_traits.h"
#include "status/status.h"
#include "status/status_or.h"
#include "strings/progmem_string_data.h"
#include "strings/string_view.h"

namespace mcucore {

template <typename KEY, size_t SIZE>
class SerialMap {
 public:
  struct Entry {
    size_t EntrySize() const { return offsetof(Entry, value) + length; }

    void SetKey(const KEY key) { memcpy(key_bytes, &key, sizeof(KEY)); }
    // template <enable_if_t<sizeof(KEY) == sizeof(uint8_t), int> = 1>
    // void SetKey(const KEY key) {
    //   memcpy(key_bytes, &key, sizeof(KEY));
    // }

    bool HasKey(const KEY key) const {
      return memcmp(key_bytes, &key, sizeof(KEY)) == 0;
    }

    KEY GetKey() const {
      KEY key;
      memcpy(&key, key_bytes, sizeof(KEY));
      return key;
    }

    template <typename T, enable_if_t<is_arithmetic<T>::value, bool> = true>
    StatusOr<T> GetValue() const {
      if (length != sizeof(T)) {
        return DataLossError();
      }
      T t;
      memcpy(&t, value, sizeof(T));
      return t;
    }

    template <typename T,
              enable_if_t<is_same<T, StringView>::value, bool> = true>
    StatusOr<T> GetValue() const {
      return StringView(reinterpret_cast<const char*>(value), length);
    }

    uint8_t key_bytes[sizeof(KEY)];
    uint8_t length;
    uint8_t value[];
  };

  // TODO(jamessynge): If asan enabled, add ctor that marks all of data_ as
  // poisoned.

  const Entry* First() const {
    if (end_ == 0) {
      return nullptr;
    } else {
      return reinterpret_cast<const Entry*>(&data_[0]);
    }
  }

  const Entry* Next(const Entry& entry) const {
    const uint8_t* data_ptr = &entry.value[0];
    const uint8_t* next_ptr = data_ptr + entry.length;
    if (next_ptr < &data_[end_]) {
      return reinterpret_cast<const Entry*>(next_ptr);
    }
    MCU_DCHECK_EQ(next_ptr, &data_[end_]);
    return nullptr;
  }

  const Entry* Find(const KEY key) const {
    for (auto* entry = First(); entry != nullptr; entry = Next(*entry)) {
      if (entry->HasKey(key)) {
        return entry;
      }
    }
    return nullptr;
  }

  // Find an entry and read its value as a type T. Returns an error if not found
  // or if not the right size.
  template <typename T>
  StatusOr<T> GetValue(const KEY key) const {
    const auto* entry = Find(key);
    if (entry == nullptr) {
      return NotFoundError();
    }
    return entry->template GetValue<T>();
  }

  // Insert an entry with the specified key and the value of specified length.
  // Returns OK if it fits in the available space, and if there isn't already an
  // entry with the same key; else returns an error.
  Status Insert(const KEY key, const uint8_t length,
                const uint8_t* const value) {
    return InsertOrAssignHelper(key, length, value, true);
  }
  Status Insert(const KEY key, const StringView value) {
    return Insert(key, value.size(), value.bytes());
  }
  template <typename T, enable_if_t<is_arithmetic<T>::value, bool> = true>
  Status Insert(const KEY key, const T value) {
    return Insert(key, sizeof(value), reinterpret_cast<const uint8_t*>(&value));
  }

  Status InsertOrAssign(const KEY key, const uint8_t length,
                        const uint8_t* const value) {
    return InsertOrAssignHelper(key, length, value, false);
  }
  Status InsertOrAssign(const KEY key, const StringView value) {
    return InsertOrAssign(key, value.size(), value.bytes());
  }
  template <typename T, enable_if_t<is_arithmetic<T>::value, bool> = true>
  Status InsertOrAssign(const KEY key, const T value) {
    return InsertOrAssign(key, sizeof(value),
                          reinterpret_cast<const uint8_t*>(&value));
  }

  // Remove the entry with the specified key. Returns true if such an entry is
  // found, else false.
  bool Remove(const KEY key) {
    Entry* entry_ptr = MutableFind(key);
    if (entry_ptr == nullptr) {
      return false;
    }
    RemoveEntry(*entry_ptr);
    return true;
  }

 private:
  Entry* MutableFind(const KEY key) { return const_cast<Entry*>(Find(key)); }

  Status InsertOrAssignHelper(const KEY key, const uint8_t length,
                              const uint8_t* const value, bool fail_if_found) {
    const auto available = SIZE - end_;
    const auto need = offsetof(Entry, value) + length;
    if (need > available) {
      // Note we're not handling the case here where the key already exists in
      // the map, and thus there might be room if we subtracted the existing
      // space used for it.
      MCU_VLOG(3) << MCU_PSD("Map too full") << MCU_NAME_VAL(length)
                  << MCU_NAME_VAL(need) << MCU_NAME_VAL(available)
                  << MCU_PSD(" offset of value=") << offsetof(Entry, value);
      return ResourceExhaustedError(MCU_PSD("Map too full"));
    }
    Entry* entry = MutableFind(key);
    if (entry != nullptr) {
      if (fail_if_found) {
        return AlreadyExistsError(MCU_PSD("Key in map"));
      }
      if (entry->length != length) {
        RemoveEntry(*entry);
        entry = nullptr;
      }
    }
    if (entry == nullptr) {
      entry = reinterpret_cast<Entry*>(&data_[end_]);
      entry->SetKey(key);
      entry->length = length;
      const auto new_end = end_ + entry->EntrySize();
      MCU_DCHECK_LE(new_end, SIZE);
      end_ = new_end;
      // TODO(jamessynge): If asan enabled, mark data after the entry as
      // unpoisoned.
    }
    memcpy(entry->value, value, length);
    return OkStatus();
  }

  void RemoveEntry(Entry& entry) {
    const auto old_end = end_;
    const auto entry_offset = OffsetOfEntry(entry);
    MCU_DCHECK_LT(entry_offset, old_end);
    const auto* next_ptr = Next(entry);
    if (next_ptr == nullptr) {
      // The easy case: there is no next entry, so we don't have any data to
      // move.
      end_ = entry_offset;
    } else {
      // Need to move later entries down. How much space do those occupy?
      const auto next_offset = OffsetOfEntry(*next_ptr);
      MCU_DCHECK_LT(entry_offset, next_offset);
      MCU_DCHECK_LT(next_offset, old_end);
      const auto remaining_entries_size = end_ - next_offset;
      memmove(&entry, next_ptr, remaining_entries_size);
      end_ -= (next_offset - entry_offset);
    }
    // TODO(jamessynge): If asan enabled, mark data after the last remaining
    // entry as poisoned.
  }

  size_t OffsetOfEntry(const Entry& entry) const {
    const uint8_t* entry_ptr = reinterpret_cast<const uint8_t*>(&entry);
    return entry_ptr - data_;
  }

  size_t end_{0};
  uint8_t data_[SIZE];

  static_assert(SIZE < numeric_limits<decltype(end_)>::max(),
                "max is reserved to mean overflow");
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_CONTAINER_SERIAL_MAP_H_
