#include "status/status_code.h"

// (Mostly) GENERATED FILE. See make_enum_to_string.py

#if MCU_HOST_TARGET
#include <ostream>  // pragma: keep standard include
#endif

#include "print/print_misc.h"
#include "print/print_to_buffer.h"
#include "strings/progmem_string_data.h"

// BEGIN_SOURCE_GENERATED_BY_MAKE_ENUM_TO_STRING

namespace mcucore {
namespace {

MCU_MAYBE_UNUSED_FUNCTION inline const __FlashStringHelper*
_ToFlashStringHelperViaSwitch(StatusCode v) {
  switch (v) {
    case StatusCode::kOk:
      return MCU_FLASHSTR("Ok");
    case StatusCode::kCancelled:
      return MCU_FLASHSTR("Cancelled");
    case StatusCode::kUnknown:
      return MCU_FLASHSTR("Unknown");
    case StatusCode::kDeadlineExceeded:
      return MCU_FLASHSTR("DeadlineExceeded");
    case StatusCode::kAlreadyExists:
      return MCU_FLASHSTR("AlreadyExists");
    case StatusCode::kResourceExhausted:
      return MCU_FLASHSTR("ResourceExhausted");
    case StatusCode::kFailedPrecondition:
      return MCU_FLASHSTR("FailedPrecondition");
    case StatusCode::kAborted:
      return MCU_FLASHSTR("Aborted");
    case StatusCode::kOutOfRange:
      return MCU_FLASHSTR("OutOfRange");
    case StatusCode::kUnimplemented:
      return MCU_FLASHSTR("Unimplemented");
    case StatusCode::kInternal:
      return MCU_FLASHSTR("Internal");
    case StatusCode::kUnavailable:
      return MCU_FLASHSTR("Unavailable");
    case StatusCode::kDataLoss:
      return MCU_FLASHSTR("DataLoss");
    case StatusCode::kInvalidArgument:
      return MCU_FLASHSTR("InvalidArgument");
    case StatusCode::kNotFound:
      return MCU_FLASHSTR("NotFound");
    case StatusCode::kUnauthorized:
      return MCU_FLASHSTR("Unauthorized");
    case StatusCode::kForbidden:
      return MCU_FLASHSTR("Forbidden");
  }
  return nullptr;
}

}  // namespace

const __FlashStringHelper* ToFlashStringHelper(StatusCode v) {
#ifdef TO_FLASH_STRING_HELPER_PREFER_SWITCH
  return _ToFlashStringHelperViaSwitch(v);
#else   // not TO_FLASH_STRING_HELPER_PREFER_SWITCH
  if (v == StatusCode::kOk) {
    return MCU_FLASHSTR("Ok");
  }
  if (v == StatusCode::kCancelled) {
    return MCU_FLASHSTR("Cancelled");
  }
  if (v == StatusCode::kUnknown) {
    return MCU_FLASHSTR("Unknown");
  }
  if (v == StatusCode::kDeadlineExceeded) {
    return MCU_FLASHSTR("DeadlineExceeded");
  }
  if (v == StatusCode::kAlreadyExists) {
    return MCU_FLASHSTR("AlreadyExists");
  }
  if (v == StatusCode::kResourceExhausted) {
    return MCU_FLASHSTR("ResourceExhausted");
  }
  if (v == StatusCode::kFailedPrecondition) {
    return MCU_FLASHSTR("FailedPrecondition");
  }
  if (v == StatusCode::kAborted) {
    return MCU_FLASHSTR("Aborted");
  }
  if (v == StatusCode::kOutOfRange) {
    return MCU_FLASHSTR("OutOfRange");
  }
  if (v == StatusCode::kUnimplemented) {
    return MCU_FLASHSTR("Unimplemented");
  }
  if (v == StatusCode::kInternal) {
    return MCU_FLASHSTR("Internal");
  }
  if (v == StatusCode::kUnavailable) {
    return MCU_FLASHSTR("Unavailable");
  }
  if (v == StatusCode::kDataLoss) {
    return MCU_FLASHSTR("DataLoss");
  }
  if (v == StatusCode::kInvalidArgument) {
    return MCU_FLASHSTR("InvalidArgument");
  }
  if (v == StatusCode::kNotFound) {
    return MCU_FLASHSTR("NotFound");
  }
  if (v == StatusCode::kUnauthorized) {
    return MCU_FLASHSTR("Unauthorized");
  }
  if (v == StatusCode::kForbidden) {
    return MCU_FLASHSTR("Forbidden");
  }
  return nullptr;
#endif  // TO_FLASH_STRING_HELPER_PREFER_SWITCH
}

size_t PrintValueTo(StatusCode v, Print& out) {
  auto flash_string = ToFlashStringHelper(v);
  if (flash_string != nullptr) {
    return out.print(flash_string);
  }
  return mcucore::PrintUnknownEnumValueTo(MCU_FLASHSTR("StatusCode"),
                                          static_cast<uint32_t>(v), out);
}

#if MCU_HOST_TARGET
// Support for debug logging of enums.

std::ostream& operator<<(std::ostream& os, StatusCode v) {
  char buffer[256];
  mcucore::PrintToBuffer print(buffer);
  PrintValueTo(v, print);
  return os << std::string_view(buffer, print.data_size());
}

#endif  // MCU_HOST_TARGET

}  // namespace mcucore

// END_SOURCE_GENERATED_BY_MAKE_ENUM_TO_STRING
