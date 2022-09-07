// Enable all of the logging features before including *any* files.
#undef MCU_DISABLE_CHECK
#undef MCU_DISABLE_CHECK_LOCATION
#undef MCU_DISABLE_DCHECK
#undef MCU_DISABLE_DCHECK_LOCATION
#undef MCU_DISABLE_VLOG
#undef MCU_DISABLE_VLOG_LOCATION

#define MCU_ENABLED_VLOG_LEVEL 5
#define MCU_ENABLE_CHECK
#define MCU_ENABLE_CHECK_LOCATION
#define MCU_ENABLE_DCHECK
#define MCU_ENABLE_DCHECK_LOCATION
#define MCU_ENABLE_VLOG
#define MCU_ENABLE_VLOG_LOCATION

#include "status/status_or.h"

#include <stdint.h>

#include "extras/test_tools/log/check_sink_test_base.h"
#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "gtest/gtest.h"
#include "status/status.h"
#include "status/status_code.h"
#include "strings/progmem_string_view.h"
#include "strings/string_view.h"

namespace mcucore {
namespace test {
namespace {

template <typename T>
StatusOr<T> ReturnStatusWithoutMessage(StatusCode code) {
  return Status(code);
}

template <typename T>
StatusOr<T> ReturnStatusWithMessage(StatusCode code,
                                    ProgmemStringView message) {
  return Status(code, message);
}

template <typename T>
StatusOr<T> ReturnValue(const T& value) {
  return value;
}

TEST(StatusOrTest, IntValue) {
  StatusOr<int> status_or(1);
  EXPECT_TRUE(status_or.ok());
  EXPECT_EQ(status_or.value(), 1);
  EXPECT_TRUE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), StatusCode::kOk);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView());

  status_or = Status(StatusCode::kInvalidArgument, ProgmemStringView("Oops"));
  EXPECT_FALSE(status_or.ok());
  EXPECT_FALSE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), StatusCode::kInvalidArgument);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView("Oops"));

  status_or = ReturnValue<int>(123);
  EXPECT_TRUE(status_or.ok());
  EXPECT_EQ(status_or.value(), 123);
  EXPECT_TRUE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), StatusCode::kOk);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView());

  status_or = ReturnStatusWithoutMessage<int>(StatusCode::kNotFound);
  EXPECT_FALSE(status_or.ok());
  EXPECT_FALSE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), StatusCode::kNotFound);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView(""));
}

TEST(StatusOrTest, StringViewValue) {
  auto status_or = ReturnValue(StringView("abc"));
  EXPECT_TRUE(status_or.ok());
  EXPECT_EQ(status_or.value(), StringView("abc"));
  EXPECT_TRUE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), StatusCode::kOk);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView());

  status_or = Status(StatusCode::kResourceExhausted);
  EXPECT_FALSE(status_or.ok());
  EXPECT_FALSE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), StatusCode::kResourceExhausted);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView(""));

  status_or = StringView();
  EXPECT_TRUE(status_or.ok());
  EXPECT_EQ(status_or.value(), StringView(""));
  EXPECT_TRUE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), StatusCode::kOk);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView());

  status_or = ReturnStatusWithMessage<StringView>(
      StatusCode::kNotFound, ProgmemStringView("where is it?"));
  EXPECT_FALSE(status_or.ok());
  EXPECT_FALSE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), StatusCode::kNotFound);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView("where is it?"));
}

TEST(StatusOrTest, Ctors) {
  {
    StatusOr<int> status_or_int(1);
    EXPECT_TRUE(status_or_int.ok());
    EXPECT_EQ(status_or_int.status(), OkStatus());
    EXPECT_EQ(status_or_int.value(), 1);
  }
  {
    StatusOr<int> status_or_int(StatusOr<int>(345));
    EXPECT_TRUE(status_or_int.ok());
    EXPECT_EQ(status_or_int.status(), OkStatus());
    EXPECT_EQ(status_or_int.value(), 345);
  }
  {
    const auto code = Status(StatusCode::kInvalidArgument);
    StatusOr<int> status_or_int(code);
    EXPECT_FALSE(status_or_int.ok());
    EXPECT_EQ(status_or_int.status(), code);
  }
}

TEST(StatusOrTest, AssignOrReturn_OKValue) {
  bool did_not_return = false;
  auto outer = [&]() -> Status {
    StatusOr<int> value = 987;
    MCU_ASSIGN_OR_RETURN(auto value_in_status_or, value);
    did_not_return = true;
    EXPECT_EQ(value_in_status_or, 987);
    return OkStatus();
  };
  EXPECT_EQ(outer(), OkStatus());
  EXPECT_TRUE(did_not_return);
}

TEST(StatusOrTest, AssignOrReturn_CallReturnsOK) {
  auto inner = []() -> StatusOr<int> { return 987; };
  bool did_not_return = false;
  auto outer = [&]() -> Status {
    MCU_ASSIGN_OR_RETURN(auto value_in_status_or, inner());
    did_not_return = true;
    EXPECT_EQ(value_in_status_or, 987);
    return OkStatus();
  };
  EXPECT_EQ(outer(), OkStatus());
  EXPECT_TRUE(did_not_return);
}

TEST(StatusOrTest, AssignOrReturn_ErrorNotValue) {
  bool did_return = false;
  auto outer = [&]() -> Status {
    StatusOr<int> status_or_value1 = Status(StatusCode::kInvalidArgument);
    StatusOr<int> status_or_value2(0);
    did_return = true;
    MCU_ASSIGN_OR_RETURN(status_or_value2, status_or_value1);
    did_return = false;
    ADD_FAILURE() << "Should have returned! status_or_value2="
                  << status_or_value2;
    return Status();
  };
  EXPECT_EQ(outer(), Status(StatusCode::kInvalidArgument));
  EXPECT_TRUE(did_return);
}

TEST(StatusOrTest, AssignOrReturn_CallReturnsError) {
  auto inner = []() -> StatusOr<int> {
    return Status(StatusCode::kInvalidArgument);
  };
  bool did_return = false;
  auto outer = [&]() -> Status {
    did_return = true;
    MCU_ASSIGN_OR_RETURN(auto value_in_status_or, inner());
    did_return = false;
    ADD_FAILURE() << "Should have returned! value_in_status_or="
                  << value_in_status_or;
    return Status();
  };
  EXPECT_EQ(outer(), Status(StatusCode::kInvalidArgument));
  EXPECT_TRUE(did_return);
}

TEST(StatusOrTest, Equals) {
  EXPECT_EQ(StatusOr<int>(), StatusOr<int>());
  EXPECT_EQ(StatusOr<int>(), StatusOr<int>(UnknownError()));
  EXPECT_EQ(StatusOr<int>(123), StatusOr<int>(123));

  EXPECT_FALSE(StatusOr<int>() == StatusOr<int>(123));
  EXPECT_FALSE(StatusOr<int>() == StatusOr<int>(AbortedError()));
  EXPECT_FALSE(StatusOr<int>(123) == StatusOr<int>(234));
}

TEST(StatusOrTest, NotEquals) {
  EXPECT_NE(StatusOr<int>(), StatusOr<int>(123));
  EXPECT_NE(StatusOr<int>(), StatusOr<int>(AbortedError()));
  EXPECT_NE(StatusOr<int>(123), StatusOr<int>(234));

  EXPECT_FALSE(StatusOr<int>() != StatusOr<int>());
  EXPECT_FALSE(StatusOr<int>() != StatusOr<int>(UnknownError()));
  EXPECT_FALSE(StatusOr<int>(123) != StatusOr<int>(123));
}

TEST(StatusOrTest, ConstOkValue) {
  const auto status_or = StatusOr<float>(1.23f);
  EXPECT_STATUS_OK(status_or);
  EXPECT_EQ(status_or.status(), OkStatus());
  EXPECT_EQ(status_or.value(), 1.23f);

  // operator const Status&()
  const auto status = [&]() -> Status { return status_or; }();
  EXPECT_STATUS_OK(status);
}

using StatusOrCheckTest = CheckSinkTestBase;

TEST_F(StatusOrCheckTest, BadCtorCall) {
  // Not OK to initialize with an OK Status.
  VerifyFailure(
      []() {
        Status ok_status = OkStatus();
        StatusOr<int> status_or_int(ok_status);
        return __LINE__ - 1;
      },
      "status_or.h", "!status_.ok()", "", /*verify_line_number=*/false);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
