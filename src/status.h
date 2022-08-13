#ifndef MCUCORE_SRC_STATUS_H_
#define MCUCORE_SRC_STATUS_H_

// This is a simplistic version of absl::Status, along with helper methods such
// as:
//   * OkStatus(), which returns a Status instance with code kOk.
//   * DataLossError(msg), which returns a Status instance with code kDataLoss,
//     and with an optional message.
//   * IsUnknown(status), which returns true IFF `status` has code kUnknown.
//   * GetStatus(status_expr), which returns the Status of the value of
//     `status_expr`. The type of expression `status_expr` must either be
//     convertible to `const Status&`, or must be a type T that has a method
//     `T::status()` whose return type is can be assigned to a Status instance.
//
// And these macros:
//   * MCU_RETURN_IF_ERROR(status_expr), which evaluates GetStatus(status_expr)
//     and if the resulting status is not OK, returns the status.
//   * MCU_CHECK_OK(status_expr), which is roughly the same as:
//         const Status temp_var_name = GetStatus(status_expr);
//         MCU_CHECK(temp_var_name.ok()) << temp_var_name
//   * MCU_DCHECK_OK(status_expr), which is like MCU_CHECK_OK, but only when
//     MCU_ENABLE_DCHECK is defined. Otherwise it is like MCU_DCHECK(true).
//
// =============================================================================
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
#include "progmem_string_view.h"
#include "semistd/type_traits.h"
#include "status_code.h"  // pragma IWYU: export

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

// TODO(jamessynge): Consider adding parameters to record the location (line
// number and file name) where the Status was created, in particular with
// defaults provided by the non-standard functions __builtin_LINE and
// __builtin_FILE.

class Status {
 public:
  // Defaults to OK. This is different than StatusOr.
  Status() : code_(StatusCode::kOk) {}
  explicit Status(StatusCode code) : code_(code) {}
  Status(StatusCode code, ProgmemStringView message)
      : code_(code),
        message_(code != StatusCode::kOk ? message : ProgmemStringView()) {}

  template <typename T, enable_if_t<has_to_status_code<T>::value>>
  explicit Status(T code_to_convert) : code_(ToStatusCode(code_to_convert)) {}

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

// `MCU_RETURN_IF_ERROR(expr)` evaluates `expr`, whose type must be convertable
// to Status, and returns the Status if it is not OK.
#define MCU_RETURN_IF_ERROR(expr) \
  MCU_RETURN_IF_ERROR_IMPL_(MAKE_UNIQUE_NAME(_return_if_error_status_), expr)

#define MCU_CHECK_OK(expr) \
  MCU_CHECK_OK_IMPL_(MAKE_UNIQUE_NAME(_check_status_ok_), expr)

#ifdef MCU_ENABLE_DCHECK
#define MCU_DCHECK_OK(expr) \
  MCU_CHECK_OK_IMPL_(MAKE_UNIQUE_NAME(_dcheck_status_ok_), expr)
#else
#define MCU_DCHECK_OK(expr) MCU_DCHECK(true)
#endif  // MCU_ENABLE_DCHECK

////////////////////////////////////////////////////////////////////////////////
// Internal macros, not for direct use.

#define MCU_RETURN_IF_ERROR_IMPL_(name, expr)     \
  do {                                            \
    const auto name = ::mcucore::GetStatus(expr); \
    if (!name.ok()) {                             \
      return name;                                \
    }                                             \
  } while (false)

#define MCU_CHECK_OK_IMPL_(name, expr)                            \
  for (const auto name = ::mcucore::GetStatus(expr); !name.ok();) \
  MCU_CHECK_INTERNAL_(name.ok(), #expr) << name

#endif  // MCUCORE_SRC_STATUS_H_
