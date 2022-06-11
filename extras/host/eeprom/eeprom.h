#ifndef MCUCORE_EXTRAS_HOST_EEPROM_EEPROM_H_
#define MCUCORE_EXTRAS_HOST_EEPROM_EEPROM_H_

// A totally fake implementation of Arduino's EEPROM API. Just stores in RAM,
// and has no persistence. IFF we really wanted to, we could use mmap to read
// and write from a file, and thus provide persistence from run to run... which
// would complicate testing; if appropriate to add this feature this, we could
// make that configurable.

#include <stdint.h>

#include <vector>

class EEPROMClass;

namespace internal {
uint8_t eeprom_read_byte(EEPROMClass& eeprom, int index);
void eeprom_write_byte(EEPROMClass& eeprom, int index, uint8_t value);
}  // namespace internal

// Simulates a reference to an EEPROM byte.
class EERef {
 public:
  EERef(EEPROMClass& eeprom, const int index)
      : eeprom_(eeprom), index_(index) {}

  // Access/read members.
  uint8_t operator*() const {
    return internal::eeprom_read_byte(eeprom_, index_);
  }
  operator uint8_t() const { return **this; }  // NOLINT

  // Assignment/write members.
  EERef& operator=(const EERef& ref) { return *this = *ref; }
  EERef& operator=(uint8_t value) {
    internal::eeprom_write_byte(eeprom_, index_, value);
    return *this;
  }

  /** Prefix increment/decrement **/
  EERef& operator++() {
    ++index_;
    return *this;
  }
  EERef& operator--() {
    --index_;
    return *this;
  }

  /** Postfix increment/decrement **/
  uint8_t operator++(int) {
    auto copy = *this;
    ++index_;
    return copy;
  }

  uint8_t operator--(int) {
    auto copy = *this;
    --index_;
    return copy;
  }

 private:
  EEPROMClass& eeprom_;
  int index_;
};

class EEPROMClass {
 public:
  static constexpr uint16_t kDefaultSize = 512;

  // This ctor doesn't exist in the normal Arduino library, but is useful for
  // testing with various sizes of fake EEPROM.
  explicit EEPROMClass(uint16_t length = kDefaultSize);
  virtual ~EEPROMClass() {}

  // Basic user access methods. Some methods are virtual to allow overriding
  // in test fixtures.
  virtual uint8_t read(int idx) { return data_[idx]; }
  virtual void write(int idx, uint8_t val) { data_[idx] = val; }
  void update(int idx, uint8_t val) { write(idx, val); }

  EERef operator[](const int idx) { return EERef(*this, idx); }

  uint16_t length() { return static_cast<uint16_t>(data_.size()); }

  // Functionality to 'get' and 'put' objects to and from EEPROM.
  template <typename T>
  T& get(int idx, T& t) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(&t);
    int limit = idx + sizeof(T);
    while (idx < limit) {
      *ptr++ = read(idx++);
    }
    return t;
  }

  template <typename T>
  const T& put(int idx, const T& t) {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&t);
    int limit = idx + sizeof(T);
    while (idx < limit) {
      update(idx++, *ptr++);
    }
    return t;
  }

 private:
  // Initializes to zeroes.
  std::vector<uint8_t> data_;
};

extern EEPROMClass EEPROM;

#endif  // MCUCORE_EXTRAS_HOST_EEPROM_EEPROM_H_
