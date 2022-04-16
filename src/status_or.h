#ifndef MCUCORE_SRC_STATUS_OR_H_
#define MCUCORE_SRC_STATUS_OR_H_

// This is a simplistic version of absl::StatusOr, supporting methods that
// need to return a value, or an error status.
//
// Author: james.synge@gmail.com

#include "logging.h"
#include "mcucore_platform.h"  // IWYU pragma: keep
#include "status.h"
#include "status_code.h"
#include "type_traits.h"

namespace mcucore {

// T can not be a class with virtual functions, nor with a destructor, because
// it must be valid as the type of a member of an anonymous union.
template <typename T>
class StatusOr {
 public:
  using value_type = typename remove_cv<T>::type;
  StatusOr() : StatusOr(Status(StatusCode::kUnknown)) {}
  /*implicit*/ StatusOr(const value_type& value)  // NOLINT
      : value_(value) {}
  /*implicit*/ StatusOr(const Status& status)  // NOLINT
      : status_(status) {
    MCU_DCHECK(!status.ok());
    if (status_.ok()) {
      status_ = Status(StatusCode::kUnknown);  // COV_NF_LINE
    }
  }

  bool ok() const { return status_.ok(); }

  const value_type& value() const {
    MCU_CHECK(ok());
    return value_;
  }

  const Status& status() const { return status_; }

  operator const Status&() const {  // NOLINT
    return status_;
  }

 private:
  Status status_;
  // Note: it could be useful to have a simple version of std::variant with
  // which to implement StatusOr.
  union {
    value_type value_;
  };
};

template <typename T>
bool operator==(const StatusOr<T>& a, const StatusOr<T>& b) {
  if (a.ok() && b.ok()) {
    return a.value() == b.value();
  } else {
    return a.status() == b.status();
  }
}
template <typename T>
bool operator!=(const StatusOr<T>& a, const StatusOr<T>& b) {
  return !(a == b);
}

}  // namespace mcucore

#define MCU_ASSIGN_OR_RETURN(lhs, status_or_expression)                \
  MCU_ASSIGN_OR_RETURN_IMPL_(                                          \
      MCU_STATUS_MACROS_CONCAT_NAME(_status_or_value_, __LINE__), lhs, \
      status_or_expression)

#define MCU_ASSIGN_OR_RETURN_IMPL_(statusor, lhs, status_or_expression) \
  auto statusor = status_or_expression;                                 \
  if (!statusor.ok()) {                                                 \
    return statusor.status();                                           \
  }                                                                     \
  lhs = statusor.value();

#endif  // MCUCORE_SRC_STATUS_OR_H_
