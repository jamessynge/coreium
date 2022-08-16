#include "status/status_code.h"

// (Mostly) GENERATED FILE. See make_enum_to_string.py

#include "container/flash_string_table.h"
#include "mcucore_platform.h"
#include "print/print_misc.h"
#include "strings/progmem_string_data.h"

namespace mcucore {
// BEGIN_SOURCE_GENERATED_BY_MAKE_ENUM_TO_STRING

const __FlashStringHelper* ToFlashStringHelper(StatusCode v) {
#ifdef TO_FLASH_STRING_HELPER_USE_SWITCH
  switch (v) {
    case StatusCode::kOk:
      return MCU_FLASHSTR("Ok");
    case StatusCode::kUnknown:
      return MCU_FLASHSTR("Unknown");
    case StatusCode::kResourceExhausted:
      return MCU_FLASHSTR("ResourceExhausted");
    case StatusCode::kFailedPrecondition:
      return MCU_FLASHSTR("FailedPrecondition");
    case StatusCode::kOutOfRange:
      return MCU_FLASHSTR("OutOfRange");
    case StatusCode::kUnimplemented:
      return MCU_FLASHSTR("Unimplemented");
    case StatusCode::kInternal:
      return MCU_FLASHSTR("Internal");
    case StatusCode::kDataLoss:
      return MCU_FLASHSTR("DataLoss");
    case StatusCode::kInvalidArgument:
      return MCU_FLASHSTR("InvalidArgument");
    case StatusCode::kNotFound:
      return MCU_FLASHSTR("NotFound");
  }
  return nullptr;
#else   // Use if statements.
  if (v == StatusCode::kOk) {
    return MCU_FLASHSTR("Ok");
  }
  if (v == StatusCode::kUnknown) {
    return MCU_FLASHSTR("Unknown");
  }
  if (v == StatusCode::kResourceExhausted) {
    return MCU_FLASHSTR("ResourceExhausted");
  }
  if (v == StatusCode::kFailedPrecondition) {
    return MCU_FLASHSTR("FailedPrecondition");
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
  if (v == StatusCode::kDataLoss) {
    return MCU_FLASHSTR("DataLoss");
  }
  if (v == StatusCode::kInvalidArgument) {
    return MCU_FLASHSTR("InvalidArgument");
  }
  if (v == StatusCode::kNotFound) {
    return MCU_FLASHSTR("NotFound");
  }
  return nullptr;
#endif  // TO_FLASH_STRING_HELPER_USE_SWITCH
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
  switch (v) {
    case StatusCode::kOk:
      return os << "Ok";
    case StatusCode::kUnknown:
      return os << "Unknown";
    case StatusCode::kResourceExhausted:
      return os << "ResourceExhausted";
    case StatusCode::kFailedPrecondition:
      return os << "FailedPrecondition";
    case StatusCode::kOutOfRange:
      return os << "OutOfRange";
    case StatusCode::kUnimplemented:
      return os << "Unimplemented";
    case StatusCode::kInternal:
      return os << "Internal";
    case StatusCode::kDataLoss:
      return os << "DataLoss";
    case StatusCode::kInvalidArgument:
      return os << "InvalidArgument";
    case StatusCode::kNotFound:
      return os << "NotFound";
  }
  // This should match the formatting by PrintUnknownEnumValueTo.
  return os << "Undefined StatusCode (" << static_cast<uint32_t>(v) << ")";
}

#endif  // MCU_HOST_TARGET

// END_SOURCE_GENERATED_BY_MAKE_ENUM_TO_STRING

}  // namespace mcucore
