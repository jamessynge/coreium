#include "status/status.h"

#include "mcucore_platform.h"
#include "print/counting_print.h"
#include "print/hex_escape.h"
#include "print/o_print_stream.h"
#include "status/status_code.h"
#include "strings/progmem_string_data.h"
#include "strings/progmem_string_view.h"

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

Status AbortedError(ProgmemStringView message) {
  return Status(StatusCode::kAborted, message);
}
Status AlreadyExistsError(ProgmemStringView message) {
  return Status(StatusCode::kAlreadyExists, message);
}
Status CancelledError(ProgmemStringView message) {
  return Status(StatusCode::kCancelled, message);
}
Status DataLossError(ProgmemStringView message) {
  return Status(StatusCode::kDataLoss, message);
}
Status DeadlineExceededError(ProgmemStringView message) {
  return Status(StatusCode::kDeadlineExceeded, message);
}
Status FailedPreconditionError(ProgmemStringView message) {
  return Status(StatusCode::kFailedPrecondition, message);
}
Status ForbiddenError(ProgmemStringView message) {
  return Status(StatusCode::kForbidden, message);
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
Status UnauthorizedError(ProgmemStringView message) {
  return Status(StatusCode::kUnauthorized, message);
}
Status UnavailableError(ProgmemStringView message) {
  return Status(StatusCode::kUnavailable, message);
}
Status UnimplementedError(ProgmemStringView message) {
  return Status(StatusCode::kUnimplemented, message);
}
Status UnknownError(ProgmemStringView message) {
  return Status(StatusCode::kUnknown, message);
}

bool IsAborted(const Status& status) {
  return status.code() == StatusCode::kAborted;
}
bool IsAlreadyExists(const Status& status) {
  return status.code() == StatusCode::kAlreadyExists;
}
bool IsCancelled(const Status& status) {
  return status.code() == StatusCode::kCancelled;
}
bool IsDataLoss(const Status& status) {
  return status.code() == StatusCode::kDataLoss;
}
bool IsDeadlineExceeded(const Status& status) {
  return status.code() == StatusCode::kDeadlineExceeded;
}
bool IsFailedPrecondition(const Status& status) {
  return status.code() == StatusCode::kFailedPrecondition;
}
bool IsForbidden(const Status& status) {
  return status.code() == StatusCode::kForbidden;
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
bool IsUnauthorized(const Status& status) {
  return status.code() == StatusCode::kUnauthorized;
}
bool IsUnavailable(const Status& status) {
  return status.code() == StatusCode::kUnavailable;
}
bool IsUnimplemented(const Status& status) {
  return status.code() == StatusCode::kUnimplemented;
}
bool IsUnknown(const Status& status) {
  return status.code() == StatusCode::kUnknown;
}
}  // namespace mcucore
