#include "http1/request_decoder_constants.h"

#if MCU_HOST_TARGET
#include <ostream>  // pragma: keep standard include
#endif

#include "container/flash_string_table.h"
#include "print/print_misc.h"
#include "print/print_to_buffer.h"
#include "strings/progmem_string_data.h"

namespace mcucore {
namespace http1 {

// BEGIN_SOURCE_GENERATED_BY_MAKE_ENUM_TO_STRING

const __FlashStringHelper* ToFlashStringHelper(EEvent v) {
#ifdef TO_FLASH_STRING_HELPER_USE_SWITCH
  switch (v) {
    case EEvent::kPathStart:
      return MCU_FLASHSTR("PathStart");
    case EEvent::kPathSeparator:
      return MCU_FLASHSTR("PathSeparator");
    case EEvent::kPathEnd:
      return MCU_FLASHSTR("PathEnd");
    case EEvent::kPathEndQueryStart:
      return MCU_FLASHSTR("PathEndQueryStart");
    case EEvent::kParamSeparator:
      return MCU_FLASHSTR("ParamSeparator");
    case EEvent::kHttpVersion1_1:
      return MCU_FLASHSTR("HttpVersion1_1");
    case EEvent::kHeadersEnd:
      return MCU_FLASHSTR("HeadersEnd");
  }
  return nullptr;
#elif defined(TO_FLASH_STRING_HELPER_PREFER_IF_STATEMENTS)
  if (v == EEvent::kPathStart) {
    return MCU_FLASHSTR("PathStart");
  }
  if (v == EEvent::kPathSeparator) {
    return MCU_FLASHSTR("PathSeparator");
  }
  if (v == EEvent::kPathEnd) {
    return MCU_FLASHSTR("PathEnd");
  }
  if (v == EEvent::kPathEndQueryStart) {
    return MCU_FLASHSTR("PathEndQueryStart");
  }
  if (v == EEvent::kParamSeparator) {
    return MCU_FLASHSTR("ParamSeparator");
  }
  if (v == EEvent::kHttpVersion1_1) {
    return MCU_FLASHSTR("HttpVersion1_1");
  }
  if (v == EEvent::kHeadersEnd) {
    return MCU_FLASHSTR("HeadersEnd");
  }
  return nullptr;
#else   // Use flash string table.
  static MCU_FLASH_STRING_TABLE(
      flash_string_table,
      MCU_FLASHSTR("PathStart"),          // 0: kPathStart
      MCU_FLASHSTR("PathSeparator"),      // 1: kPathSeparator
      MCU_FLASHSTR("PathEnd"),            // 2: kPathEnd
      MCU_FLASHSTR("PathEndQueryStart"),  // 3: kPathEndQueryStart
      MCU_FLASHSTR("ParamSeparator"),     // 4: kParamSeparator
      MCU_FLASHSTR("HttpVersion1_1"),     // 5: kHttpVersion1_1
      MCU_FLASHSTR("HeadersEnd"),         // 6: kHeadersEnd
  );
  return LookupFlashStringForDenseEnum<uint_fast8_t>(
      flash_string_table, EEvent::kPathStart, EEvent::kHeadersEnd, v);
#endif  // TO_FLASH_STRING_HELPER_USE_SWITCH
}

const __FlashStringHelper* ToFlashStringHelper(EToken v) {
#ifdef TO_FLASH_STRING_HELPER_USE_SWITCH
  switch (v) {
    case EToken::kHttpMethod:
      return MCU_FLASHSTR("HttpMethod");
    case EToken::kPathSegment:
      return MCU_FLASHSTR("PathSegment");
    case EToken::kParamName:
      return MCU_FLASHSTR("ParamName");
    case EToken::kParamValue:
      return MCU_FLASHSTR("ParamValue");
    case EToken::kHeaderName:
      return MCU_FLASHSTR("HeaderName");
    case EToken::kHeaderValue:
      return MCU_FLASHSTR("HeaderValue");
  }
  return nullptr;
#elif defined(TO_FLASH_STRING_HELPER_PREFER_IF_STATEMENTS)
  if (v == EToken::kHttpMethod) {
    return MCU_FLASHSTR("HttpMethod");
  }
  if (v == EToken::kPathSegment) {
    return MCU_FLASHSTR("PathSegment");
  }
  if (v == EToken::kParamName) {
    return MCU_FLASHSTR("ParamName");
  }
  if (v == EToken::kParamValue) {
    return MCU_FLASHSTR("ParamValue");
  }
  if (v == EToken::kHeaderName) {
    return MCU_FLASHSTR("HeaderName");
  }
  if (v == EToken::kHeaderValue) {
    return MCU_FLASHSTR("HeaderValue");
  }
  return nullptr;
#else   // Use flash string table.
  static MCU_FLASH_STRING_TABLE(flash_string_table,
                                MCU_FLASHSTR("HttpMethod"),   // 0: kHttpMethod
                                MCU_FLASHSTR("PathSegment"),  // 1: kPathSegment
                                MCU_FLASHSTR("ParamName"),    // 2: kParamName
                                MCU_FLASHSTR("ParamValue"),   // 3: kParamValue
                                MCU_FLASHSTR("HeaderName"),   // 4: kHeaderName
                                MCU_FLASHSTR("HeaderValue"),  // 5: kHeaderValue
  );
  return LookupFlashStringForDenseEnum<uint_fast8_t>(
      flash_string_table, EToken::kHttpMethod, EToken::kHeaderValue, v);
#endif  // TO_FLASH_STRING_HELPER_USE_SWITCH
}

const __FlashStringHelper* ToFlashStringHelper(EPartialToken v) {
#ifdef TO_FLASH_STRING_HELPER_USE_SWITCH
  switch (v) {
    case EPartialToken::kPathSegment:
      return MCU_FLASHSTR("PathSegment");
    case EPartialToken::kParamName:
      return MCU_FLASHSTR("ParamName");
    case EPartialToken::kParamValue:
      return MCU_FLASHSTR("ParamValue");
    case EPartialToken::kRawQueryString:
      return MCU_FLASHSTR("RawQueryString");
    case EPartialToken::kHeaderName:
      return MCU_FLASHSTR("HeaderName");
    case EPartialToken::kHeaderValue:
      return MCU_FLASHSTR("HeaderValue");
  }
  return nullptr;
#elif defined(TO_FLASH_STRING_HELPER_PREFER_IF_STATEMENTS)
  if (v == EPartialToken::kPathSegment) {
    return MCU_FLASHSTR("PathSegment");
  }
  if (v == EPartialToken::kParamName) {
    return MCU_FLASHSTR("ParamName");
  }
  if (v == EPartialToken::kParamValue) {
    return MCU_FLASHSTR("ParamValue");
  }
  if (v == EPartialToken::kRawQueryString) {
    return MCU_FLASHSTR("RawQueryString");
  }
  if (v == EPartialToken::kHeaderName) {
    return MCU_FLASHSTR("HeaderName");
  }
  if (v == EPartialToken::kHeaderValue) {
    return MCU_FLASHSTR("HeaderValue");
  }
  return nullptr;
#else   // Use flash string table.
  static MCU_FLASH_STRING_TABLE(
      flash_string_table,
      MCU_FLASHSTR("PathSegment"),     // 0: kPathSegment
      MCU_FLASHSTR("ParamName"),       // 1: kParamName
      MCU_FLASHSTR("ParamValue"),      // 2: kParamValue
      MCU_FLASHSTR("RawQueryString"),  // 3: kRawQueryString
      MCU_FLASHSTR("HeaderName"),      // 4: kHeaderName
      MCU_FLASHSTR("HeaderValue"),     // 5: kHeaderValue
  );
  return LookupFlashStringForDenseEnum<uint_fast8_t>(
      flash_string_table, EPartialToken::kPathSegment,
      EPartialToken::kHeaderValue, v);
#endif  // TO_FLASH_STRING_HELPER_USE_SWITCH
}

const __FlashStringHelper* ToFlashStringHelper(EPartialTokenPosition v) {
#ifdef TO_FLASH_STRING_HELPER_USE_SWITCH
  switch (v) {
    case EPartialTokenPosition::kFirst:
      return MCU_FLASHSTR("First");
    case EPartialTokenPosition::kMiddle:
      return MCU_FLASHSTR("Middle");
    case EPartialTokenPosition::kLast:
      return MCU_FLASHSTR("Last");
  }
  return nullptr;
#elif defined(TO_FLASH_STRING_HELPER_PREFER_IF_STATEMENTS)
  if (v == EPartialTokenPosition::kFirst) {
    return MCU_FLASHSTR("First");
  }
  if (v == EPartialTokenPosition::kMiddle) {
    return MCU_FLASHSTR("Middle");
  }
  if (v == EPartialTokenPosition::kLast) {
    return MCU_FLASHSTR("Last");
  }
  return nullptr;
#else   // Use flash string table.
  static MCU_FLASH_STRING_TABLE(flash_string_table,
                                MCU_FLASHSTR("First"),   // 0: kFirst
                                MCU_FLASHSTR("Middle"),  // 1: kMiddle
                                MCU_FLASHSTR("Last"),    // 2: kLast
  );
  return LookupFlashStringForDenseEnum<uint_fast8_t>(
      flash_string_table, EPartialTokenPosition::kFirst,
      EPartialTokenPosition::kLast, v);
#endif  // TO_FLASH_STRING_HELPER_USE_SWITCH
}

const __FlashStringHelper* ToFlashStringHelper(EDecodeBufferStatus v) {
#ifdef TO_FLASH_STRING_HELPER_USE_SWITCH
  switch (v) {
    case EDecodeBufferStatus::kDecodingInProgress:
      return MCU_FLASHSTR("DecodingInProgress");
    case EDecodeBufferStatus::kNeedMoreInput:
      return MCU_FLASHSTR("NeedMoreInput");
    case EDecodeBufferStatus::kComplete:
      return MCU_FLASHSTR("Complete");
    case EDecodeBufferStatus::kLastOkStatus:
      return MCU_FLASHSTR("LastOkStatus");
    case EDecodeBufferStatus::kIllFormed:
      return MCU_FLASHSTR("IllFormed");
    case EDecodeBufferStatus::kInternalError:
      return MCU_FLASHSTR("InternalError");
  }
  return nullptr;
#else   // Use if statements.
  if (v == EDecodeBufferStatus::kDecodingInProgress) {
    return MCU_FLASHSTR("DecodingInProgress");
  }
  if (v == EDecodeBufferStatus::kNeedMoreInput) {
    return MCU_FLASHSTR("NeedMoreInput");
  }
  if (v == EDecodeBufferStatus::kComplete) {
    return MCU_FLASHSTR("Complete");
  }
  if (v == EDecodeBufferStatus::kLastOkStatus) {
    return MCU_FLASHSTR("LastOkStatus");
  }
  if (v == EDecodeBufferStatus::kIllFormed) {
    return MCU_FLASHSTR("IllFormed");
  }
  if (v == EDecodeBufferStatus::kInternalError) {
    return MCU_FLASHSTR("InternalError");
  }
  return nullptr;
#endif  // TO_FLASH_STRING_HELPER_USE_SWITCH
}

size_t PrintValueTo(EEvent v, Print& out) {
  auto flash_string = ToFlashStringHelper(v);
  if (flash_string != nullptr) {
    return out.print(flash_string);
  }
  return PrintUnknownEnumValueTo(MCU_FLASHSTR("EEvent"),
                                 static_cast<uint32_t>(v), out);
}

size_t PrintValueTo(EToken v, Print& out) {
  auto flash_string = ToFlashStringHelper(v);
  if (flash_string != nullptr) {
    return out.print(flash_string);
  }
  return PrintUnknownEnumValueTo(MCU_FLASHSTR("EToken"),
                                 static_cast<uint32_t>(v), out);
}

size_t PrintValueTo(EPartialToken v, Print& out) {
  auto flash_string = ToFlashStringHelper(v);
  if (flash_string != nullptr) {
    return out.print(flash_string);
  }
  return PrintUnknownEnumValueTo(MCU_FLASHSTR("EPartialToken"),
                                 static_cast<uint32_t>(v), out);
}

size_t PrintValueTo(EPartialTokenPosition v, Print& out) {
  auto flash_string = ToFlashStringHelper(v);
  if (flash_string != nullptr) {
    return out.print(flash_string);
  }
  return PrintUnknownEnumValueTo(MCU_FLASHSTR("EPartialTokenPosition"),
                                 static_cast<uint32_t>(v), out);
}

size_t PrintValueTo(EDecodeBufferStatus v, Print& out) {
  auto flash_string = ToFlashStringHelper(v);
  if (flash_string != nullptr) {
    return out.print(flash_string);
  }
  return PrintUnknownEnumValueTo(MCU_FLASHSTR("EDecodeBufferStatus"),
                                 static_cast<uint32_t>(v), out);
}

#if MCU_HOST_TARGET
// Support for debug logging of enums.

std::ostream& operator<<(std::ostream& os, EEvent v) {
  char buffer[256];
  mcucore::PrintToBuffer print(buffer);
  PrintValueTo(v, print);
  return os << std::string_view(buffer, print.data_size());
}

std::ostream& operator<<(std::ostream& os, EToken v) {
  char buffer[256];
  mcucore::PrintToBuffer print(buffer);
  PrintValueTo(v, print);
  return os << std::string_view(buffer, print.data_size());
}

std::ostream& operator<<(std::ostream& os, EPartialToken v) {
  char buffer[256];
  mcucore::PrintToBuffer print(buffer);
  PrintValueTo(v, print);
  return os << std::string_view(buffer, print.data_size());
}

std::ostream& operator<<(std::ostream& os, EPartialTokenPosition v) {
  char buffer[256];
  mcucore::PrintToBuffer print(buffer);
  PrintValueTo(v, print);
  return os << std::string_view(buffer, print.data_size());
}

std::ostream& operator<<(std::ostream& os, EDecodeBufferStatus v) {
  char buffer[256];
  mcucore::PrintToBuffer print(buffer);
  PrintValueTo(v, print);
  return os << std::string_view(buffer, print.data_size());
}

#endif  // MCU_HOST_TARGET

// END_SOURCE_GENERATED_BY_MAKE_ENUM_TO_STRING

}  // namespace http1
}  // namespace mcucore
