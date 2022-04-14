#ifndef MCUCORE_SRC_STATUS_H_
#define MCUCORE_SRC_STATUS_H_

// This is a simplistic version of absl::Status.
//
// Since StatusCode is defined in the library McuCore, not in the (possibly
// multiple) libraries which may use Status, we allow for 'automatically'
// converting values of an enum `MyLibraryStatusCode` to StatusCode, by way of
// a function ToStatusCode defined (roughly) follows:
//
//    StatusCode ToStatusCode(MyLibraryStatusCode code) {
//      // Some conversion, such as a static_cast. Beware of collisions.
//      return static_cast<MyLibraryStatusCode>(code);
//    }
//
// Author: james.synge@gmail.com

#include "logging.h"
#include "mcucore_platform.h"
#include "progmem_string_data.h"
#include "progmem_string_view.h"
#include "status_code.h"  // pragma IWYU: export
#include "type_traits.h"

namespace mcucore {

// TODO(jamessynge): Consider adding an 'error code space' to Status so that we
// can support two or three spaces: ASCOM error codes, HTTP status codes, and
// possibly Unix errno values.
//
// Doing so could help avoid the need for the switch statement in
// WriteResponse::HttpErrorResponse, which has the effect of causing all of the
// error literals to be linked in to the binary if HttpErrorResponse is also
// linked in. Instead we could have a separate function for each HTTP status
// code, like those in ascom_error_codes.* for ASCOM error codes.
//
// Alternately, I could introduce HttpStatus, encapsulating an EHttpStatusCode
// and a message, which could in turn be used for producing a Status instance
// where appropriate.

class Status {
 public:
  // Defaults to OK. This is different than StatusOr.
  Status() : code_(StatusCode::kOk) {}
  explicit Status(StatusCode code) : code_(code) {
    MCU_VLOG(9) << MCU_FLASHSTR("Creating from StatusCode");
  }
  Status(StatusCode code, ProgmemStringView message)
      : code_(code),
        message_(code != StatusCode::kOk ? message : ProgmemStringView()) {}

  template <typename T, enable_if_t<has_to_status_code<T>::value>>
  explicit Status(T code_to_convert) : code_(ToStatusCode(code_to_convert)) {
    MCU_VLOG(1) << MCU_FLASHSTR("Converting value to StatusCode");
  }

  template <typename T, enable_if_t<has_to_status_code<T>::value>>
  explicit Status(T code_to_convert, ProgmemStringView message)
      : code_(ToStatusCode(code_to_convert)),
        message_(code_ != StatusCode::kOk ? message : ProgmemStringView()) {}

  bool ok() const { return code_ == StatusCode::kOk; }
  StatusCode code() const { return code_; }
  const ProgmemStringView message() const { return message_; }
  size_t printTo(Print& out) const;

 private:
  StatusCode code_;
  ProgmemStringView message_;
};

// Compare two Status instances for equality.
bool operator==(const Status& a, const Status& b);

inline Status OkStatus() { return Status(); }

// These convenience functions create a `Status` object with an error
// code as indicated by the associated function name, using the error message
// passed in `message`.
Status DataLossError(ProgmemStringView message = {});
Status FailedPreconditionError(ProgmemStringView message = {});
Status InternalError(ProgmemStringView message = {});
Status InvalidArgumentError(ProgmemStringView message = {});
Status NotFoundError(ProgmemStringView message = {});
Status OutOfRangeError(ProgmemStringView message = {});
Status ResourceExhaustedError(ProgmemStringView message = {});
Status UnimplementedError(ProgmemStringView message = {});
Status UnknownError(ProgmemStringView message = {});

// These convenience functions return `true` if a given status matches the
// `StatusCode` error code of its associated function.
bool IsDataLoss(const Status& status);
bool IsFailedPrecondition(const Status& status);
bool IsInternal(const Status& status);
bool IsInvalidArgument(const Status& status);
bool IsNotFound(const Status& status);
bool IsOutOfRange(const Status& status);
bool IsResourceExhausted(const Status& status);
bool IsUnimplemented(const Status& status);
bool IsUnknown(const Status& status);

// Support for checking the status of Status or of StatusOr<T>.
inline const Status& GetStatus(const Status& status) { return status; }

// GetStatus from a type T which has a method like `status()` returning a Status
// value or reference. Most obviously applies to StatusOr.
template <class T,
          typename enable_if<
              is_union_or_class<T>::value &&
                  is_same<Status, remove_cv_t<remove_reference_t<
                                      decltype(declval<T>().status())>>>::value,
              int>::type I = 0>
inline const Status& GetStatus(const T& status_source) {
  return status_source.status();
}

}  // namespace mcucore

// Evaluate expression, whose type must be convertable to Status, and return the
// Status if it is not OK.
#define MCU_RETURN_IF_ERROR(expr)             \
  do {                                        \
    auto status = ::mcucore::GetStatus(expr); \
    if (!status.ok()) {                       \
      return status;                          \
    }                                         \
  } while (false)

#define MCU_CHECK_OK(expr)                                     \
  for (const ::mcucore::Status status = (expr); !status.ok();) \
  MCU_CHECK_INTERNAL_(status.ok(), #expr) << status

#ifdef MCU_ENABLE_DCHECK
#define MCU_DCHECK_OK(expr) MCU_CHECK_OK(expr)
#else
#define MCU_DCHECK_OK(expr)
#endif  // MCU_ENABLE_DCHECK

// Internal helper for concatenating macro values.
#define MCU_STATUS_MACROS_CONCAT_NAME_INNER_(x, y) x##y
#define MCU_STATUS_MACROS_CONCAT_NAME(x, y) \
  MCU_STATUS_MACROS_CONCAT_NAME_INNER_(x, y)

#endif  // MCUCORE_SRC_STATUS_H_
