#ifndef MCUCORE_SRC_STATUS_CODE_H_
#define MCUCORE_SRC_STATUS_CODE_H_

// StatusCode provides common statuses to be conveyed using Status and StatusOr.
//
// To avoid defining here all status codes needed by libraries using McuCore,
// additional values may be used (e.g. static_cast<StatusCode>(other)), though I
// don't currently provide a collision detection mechanism.
//
// TODO(jamessynge): Consider using the same approach as in eeprom_domain.h to
// register named codes, and thus to detect collisions at link time; maybe also
// provide a tool for reading all source files and building a map from name to
// value.

#include "mcucore_platform.h"
#include "type_traits.h"

namespace mcucore {

// So far all the codes I've needed fit into 16 bits; if more bits are needed,
// change the size.
//
// Values under 100 are used for most status codes that don't map to HTTP codes;
// except for kOk, the specific value doesn't matter, except for the desire to
// avoid collisions.
//
// As of March, 2022, ASCOM errors are in the range 0x400 to 0x4FF (1024 to
// 1279), which should be avoided here, except where they mean the same thing.
enum class StatusCode : int16_t {
  // This must be zero so that the default value is the default value for
  // integer fields.
  kOk = 0,

  // kUnknown is the status of a default initialized StatusOr instance.
  kUnknown = 1,

  // Ran out of something we need (e.g. have an empty buffer when reading, or
  // a full buffer when writing).
  kResourceExhausted = 8,

  //////////////////////////////////////////////////////////////////////////////
  // These map to HTTP error codes:
  kInvalidArgument = 400,  // Equivalent to BAD REQUEST.

  kNotFound = 404,
};

// has_to_status_code extends either true_type or false_type, depending on
// whether there T is an enum type for which there exists a function:
//
//      StatusCode ToStatusCode(T)
//
// The first definition matches any type for which there is not a corresponding
// ToStatusCode function.
template <class, class = void>
struct has_to_status_code : false_type {};

// The second definition matches an enum type T for which there is a
// ToStatusCode(T) function, and the return type is StatusCode.
template <class T>
struct has_to_status_code<
    T, void_t<enable_if_t<
           is_enum<T>::value &&
           is_same<StatusCode, decltype(ToStatusCode(T{}))>::value>>>
    : true_type {};

}  // namespace mcucore

#endif  // MCUCORE_SRC_STATUS_CODE_H_
