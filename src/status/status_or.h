#ifndef MCUCORE_SRC_STATUS_STATUS_OR_H_
#define MCUCORE_SRC_STATUS_STATUS_OR_H_

// This is a simplistic version of absl::StatusOr, supporting methods that
// need to return a value, or an error status. Also defines the macro
// MCU_ASSIGN_OR_RETURN, which helps to streamline the handling of errors when
// calling a function Foo that returns a StatusOr<T>. For example:
//
//     MCU_ASSIGN_OR_RETURN(auto value, Foo());
//
// The code following that statement can use value, knowing that if Foo()
// returned a non-OK status, the macro will have already performed a return.
//
// Author: james.synge@gmail.com

#include "log/log.h"
#include "mcucore_platform.h"  // IWYU pragma: keep
#include "semistd/type_traits.h"
#include "status/status.h"
#include "status/status_code.h"

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
    MCU_DCHECK(!status_.ok());
    if (status_.ok()) {
      status_ = Status(StatusCode::kUnknown);
    }
  }

  ~StatusOr() {
    // To handle the case where the value_type has a non-trivial destructor, we
    // need an explicit destructor here. It is possible that by adding an
    // implementation of is_trivially_destructible, we could choose at compile
    // type a wrapper class for value_ that has this explicit call only when
    // necessary. However, if the compiler optimizes away or inlines the dtor
    // when it does nothing, that extra development time wouldn't be valuable.
    if (ok()) {
      value_.~value_type();
    }
  }

  bool ok() const { return status_.ok(); }

  const value_type& value() const {
    MCU_CHECK(ok());
    return value_;
  }

  value_type& value() {
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

#define MCU_ASSIGN_OR_RETURN(lhs, status_or_expression)                  \
  MCU_ASSIGN_OR_RETURN_IMPL_(MCU_PP_UNIQUE_NAME(_status_or_value_), lhs, \
                             status_or_expression)

#define MCU_ASSIGN_OR_RETURN_IMPL_(statusor, lhs, status_or_expression) \
  auto statusor = status_or_expression;                                 \
  if (!statusor.ok()) {                                                 \
    return statusor.status();                                           \
  }                                                                     \
  lhs = statusor.value();

#define MCU_CHECK_OK_AND_ASSIGN(lhs, status_or_expression)                  \
  MCU_CHECK_OK_AND_ASSIGN_IMPL_(MCU_PP_UNIQUE_NAME(_status_or_value_), lhs, \
                                status_or_expression)

#define MCU_CHECK_OK_AND_ASSIGN_IMPL_(statusor, lhs, status_or_expression) \
  auto statusor = status_or_expression;                                    \
  MCU_CHECK_OK(statusor);                                                  \
  lhs = statusor.value();

#endif  // MCUCORE_SRC_STATUS_STATUS_OR_H_
