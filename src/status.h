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

#include "mcucore_platform.h"
#include "progmem_string_view.h"
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

}  // namespace mcucore

// Evaluate expression, whose type must be Status, and return the Status if it
// is not OK.
#define MCU_RETURN_IF_ERROR(expr)            \
  do {                                       \
    const ::mcucore::Status status = (expr); \
    if (!status.ok()) {                      \
      return status;                         \
    }                                        \
  } while (false)

#endif  // MCUCORE_SRC_STATUS_H_
