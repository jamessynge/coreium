#ifndef MCUCORE_SRC_EEPROM_DOMAIN_H_
#define MCUCORE_SRC_EEPROM_DOMAIN_H_

// EepromDomain supports EepromTlv and EepromTag, providing the ability to
// register unique domain values, and get build-time support for detecting when
// two domains are mistakenly registered with the same name or value.
//
// The `domain` field of an EepromTag is used to provide each subsystem (e.g. a
// network driver) with a unique identifier (or multiple, if necessary), that is
// distinct from the domain used by all other pieces of code. To enable a
// subsystem to identify multiple items of data that aren't stored and loaded
// together (e.g. when some attributes are optional), an EepromTag has an `id`
// field for that purpose.
//
// To define a domain N (where N is a C++ decimal literal, 1 to 255), put the
// following in the source (not header) file of the code that uses EepromTlv, at
// file scope (i.e. not inside a namespace):
//
//     #include <McuCore.h>
//     MCU_DEFINE_DOMAIN(N);
//
// Or:
//
//     #include <McuCore.h>
//     MCU_DEFINE_NAMED_DOMAIN(MyDomainName, N);
//
// When `MyDomainName` is a unique, unqualified C++ identifier for your domain.
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
// If more than one source file is likely to need to use this domain, add the
// following to a shared header file, again at file scope:
//
//     #include <McuCore.h>
//     MCU_DECLARE_DOMAIN(N);
//
// Or:
//
//     #include <McuCore.h>
//     MCU_DECLARE_NAMED_DOMAIN(MyDomainName, N);
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

}  // namespace mcucore

// TODO(jamessynge): Add a static_assert or similar to confirm that DOMAIN is
// a decimal literal, i.e. one that starts with a 1-9, except for the DOMAIN
// zero, which is reserved to mean free.

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

#endif  // MCUCORE_SRC_EEPROM_DOMAIN_H_
