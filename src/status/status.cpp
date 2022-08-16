#include "status/status.h"

#include "mcucore_platform.h"
#include "print/counting_print.h"
#include "print/hex_escape.h"
#include "print/o_print_stream.h"
#include "strings/progmem_string_data.h"

namespace mcucore {

size_t Status::printTo(Print& out) const {
  CountingPrint counter(out);
  OPrintStream strm(counter);
  if (ok()) {
    strm << MCU_PSD("OK");
  } else {
    strm << MCU_PSD("{.code=") << code_;
    if (message_.size()) {
      strm << MCU_PSD(", .message=") << HexEscaped(message_);
    }
    strm << '}';
  }
  return counter.count();
}

bool operator==(const Status& a, const Status& b) {
  return a.code() == b.code() && a.message() == b.message();
}

bool IsDataLoss(const Status& status) {
  return status.code() == StatusCode::kDataLoss;
}
bool IsFailedPrecondition(const Status& status) {
  return status.code() == StatusCode::kFailedPrecondition;
}
bool IsInternal(const Status& status) {
  return status.code() == StatusCode::kInternal;
}
bool IsInvalidArgument(const Status& status) {
  return status.code() == StatusCode::kInvalidArgument;
}
bool IsNotFound(const Status& status) {
  return status.code() == StatusCode::kNotFound;
}
bool IsOutOfRange(const Status& status) {
  return status.code() == StatusCode::kOutOfRange;
}
bool IsResourceExhausted(const Status& status) {
  return status.code() == StatusCode::kResourceExhausted;
}
bool IsUnimplemented(const Status& status) {
  return status.code() == StatusCode::kUnimplemented;
}
bool IsUnknown(const Status& status) {
  return status.code() == StatusCode::kUnknown;
}

Status DataLossError(ProgmemStringView message) {
  return Status(StatusCode::kDataLoss, message);
}
Status FailedPreconditionError(ProgmemStringView message) {
  return Status(StatusCode::kFailedPrecondition, message);
}
Status InternalError(ProgmemStringView message) {
  return Status(StatusCode::kInternal, message);
}
Status InvalidArgumentError(ProgmemStringView message) {
  return Status(StatusCode::kInvalidArgument, message);
}
Status NotFoundError(ProgmemStringView message) {
  return Status(StatusCode::kNotFound, message);
}
Status OutOfRangeError(ProgmemStringView message) {
  return Status(StatusCode::kOutOfRange, message);
}
Status ResourceExhaustedError(ProgmemStringView message) {
  return Status(StatusCode::kResourceExhausted, message);
}
Status UnimplementedError(ProgmemStringView message) {
  return Status(StatusCode::kUnimplemented, message);
}
Status UnknownError(ProgmemStringView message) {
  return Status(StatusCode::kUnknown, message);
}

}  // namespace mcucore
