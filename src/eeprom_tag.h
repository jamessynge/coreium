#ifndef MCUCORE_SRC_EEPROM_TAG_H_
#define MCUCORE_SRC_EEPROM_TAG_H_

// EepromTag, and its contained EepromDomain, is used to identify entries in
// EEPROM, as managed by EepromTlv. The `domain` field of an EepromTag is used
// to provide each subsystem (e.g. a network driver or instance of an ASCOM
// Alpaca device) with a unique identifier (or multiple, if necessary), that is
// distinct from the domain used by all other pieces of code. To enable a
// subsystem to identify multiple items of data that aren't stored and loaded
// together (e.g. when some attributes are optional), an EepromTag has an `id`
// field for that purpose.
//
// Author: james.synge@gmail.com
//
// =============================================================================
//
// To define a domain N (where N is a C++ decimal literal, 1 to 255), put the
// following in the SOURCE file (not header file) of the code that creates
// EepromTags with domain N, and do so at file scope (i.e. not inside a
// namespace):
//
//     #include <McuCore.h>
//     MCU_DEFINE_DOMAIN(N);
//
// Or:
//
//     #include <McuCore.h>
//     MCU_DEFINE_NAMED_DOMAIN(MyDomainName, N);
//
// Where `MyDomainName` is a unique, unqualified C++ identifier for your domain.
//
// To create an instance of EepromDomain (i.e. for creating an EepromTag), use
// the following expression:
//
//     MCU_DOMAIN(N)
//
// Or:
//
//     MCU_DOMAIN(MyDomainName)
//
// If more than one source file is likely to need to create instances of this
// domain, add the following to a shared header file, again at file scope:
//
//     #include <McuCore.h>
//     MCU_DECLARE_DOMAIN(N);
//
// Or:
//
//     #include <McuCore.h>
//     MCU_DECLARE_NAMED_DOMAIN(MyDomainName, N);
//
// Alternately, you can provide an EepromTag factory function via a header:
//
//     #include <McuCore.h>
//     namespace my_ns {
//     mcucore::EepromTag MakeMyFirstTag();
//     mcucore::EepromTag MakeMyTag(uint8_t id);
//     }
//
// =============================================================================
// ============================ COLLISION DETECTION ============================
// =============================================================================
//
// The MCU_DEFINE_DOMAIN macro is designed to help with detecting (at link time)
// when two source files accidently use the same domain number. If this happens,
// you should see an error when the program is linked together; for example, if
// two source files define the domain 17, you might see this error:
//
//    ld: error: duplicate symbol: _MakeEepromDomain_17_CollisionDetector()

#include "logging.h"
#include "mcucore_platform.h"

namespace mcucore {
namespace internal {

class EepromDomain {
 public:
  EepromDomain() = delete;
  EepromDomain(const EepromDomain& other) = default;
  EepromDomain& operator=(const EepromDomain& other) = default;

  constexpr uint8_t value() const { return value_; }

  bool operator==(const EepromDomain& other) const {
    return value_ == other.value_;
  }
  bool operator!=(const EepromDomain& other) const {
    return value_ != other.value_;
  }

 private:
  constexpr explicit EepromDomain(uint8_t value) : value_(value) {}
  friend EepromDomain MakeEepromDomain(uint8_t);
  uint8_t value_;
};

/////////////////////////////////////////////////////
// Do not call this directly, use MCU_DOMAIN instead.
inline EepromDomain MakeEepromDomain(uint8_t value) {
  return EepromDomain(value);
}
/////////////////////////////////////////////////////

}  // namespace internal

using EepromDomain = internal::EepromDomain;

// Returns true if the domain is reserved (i.e. is domain 0 or 255).
bool IsReservedDomain(const EepromDomain domain);

struct EepromTag {
  // // There are reserved EepromDomains to indicate that an entry is unused.
  // bool IsUnused() const;

  // Insert formatted representation of this tag into the stream, e.g. for
  // logging.
  void InsertInto(OPrintStream& strm) const;

  // Domains are allocated to libraries or device drivers, ids are allocated
  // within a domain. Domains zero and 255 are reserved.
  EepromDomain domain;

  // The choice of id is up to the code that owns the domain (e.g. a library).
  // Some libraries may need only a single entry per domain, in which case only
  // a single id will be used; other libraries may use the id to represent an
  // index (e.g. the entry with id N represents data for some I/O channel N).
  uint8_t id;
};

// Returns true if the two tags are identical.
bool operator==(const EepromTag& lhs, const EepromTag& rhs);
inline bool operator!=(const EepromTag& a, const EepromTag& b) {
  return !(a == b);
}

}  // namespace mcucore

// TODO(jamessynge): Figure out how to add a static_assert or similar to confirm
// that DOMAIN is a decimal literal, i.e. one that starts with a 1-9, except for
// the DOMAIN zero, which is reserved to mean free. Why? To prevent two source
// files from both declaring domain 1, one file using `1` as the value and the
// other file using `0x1`, `01` or `0b1`, all of which are equivalent but would
// result in a different CollisionDetector function name, and thus the collision
// wouldn't be detected.

#define MCU_DECLARE_DOMAIN(DOMAIN) \
  ::mcucore::EepromDomain _MakeEepromDomain_##DOMAIN##_CollisionDetector()

#define MCU_DECLARE_NAMED_DOMAIN(NAME, DOMAIN) \
  MCU_DECLARE_DOMAIN(DOMAIN);                  \
  MCU_DECLARE_DOMAIN(NAME)

#define _MCU_DEFINE_DOMAIN_INNER(NAME, DOMAIN)                             \
  ::mcucore::EepromDomain _MakeEepromDomain_##NAME##_CollisionDetector() { \
    return ::mcucore::internal::MakeEepromDomain(DOMAIN);                  \
  }

#define MCU_DEFINE_DOMAIN(DOMAIN) _MCU_DEFINE_DOMAIN_INNER(DOMAIN, DOMAIN)

// For now this is the same as MCU_DEFINE_DOMAIN.
#define MCU_DEFINE_NAMED_DOMAIN(NAME, DOMAIN) \
  MCU_DEFINE_DOMAIN(DOMAIN);                  \
  _MCU_DEFINE_DOMAIN_INNER(NAME, DOMAIN)

#define MCU_DOMAIN(DOMAIN) (::_MakeEepromDomain_##DOMAIN##_CollisionDetector())

#endif  // MCUCORE_SRC_EEPROM_TAG_H_
