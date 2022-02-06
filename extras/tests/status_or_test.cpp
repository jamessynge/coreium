#include "status_or.h"

#include <stdint.h>

#include "extras/test_tools/status_or_test_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "gtest/gtest.h"
#include "progmem_string_view.h"
#include "status.h"
#include "string_view.h"

namespace mcucore {
namespace test {
namespace {

template <typename T>
StatusOr<T> ReturnStatusWithoutMessage(uint32_t code) {
  return Status(code);
}

template <typename T>
StatusOr<T> ReturnStatusWithMessage(uint32_t code, ProgmemStringView message) {
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
  EXPECT_EQ(status_or.status().code(), 0);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView());

  status_or = Status(123, ProgmemStringView("Oops"));
  EXPECT_FALSE(status_or.ok());
  EXPECT_FALSE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), 123);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView("Oops"));

  status_or = ReturnValue<int>(123);
  EXPECT_TRUE(status_or.ok());
  EXPECT_EQ(status_or.value(), 123);
  EXPECT_TRUE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), 0);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView());

  status_or = ReturnStatusWithoutMessage<int>(11508);
  EXPECT_FALSE(status_or.ok());
  EXPECT_FALSE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), 11508);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView(""));
}

TEST(StatusOrTest, StringViewValue) {
  auto status_or = ReturnValue(StringView("abc"));
  EXPECT_TRUE(status_or.ok());
  EXPECT_EQ(status_or.value(), StringView("abc"));
  EXPECT_TRUE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), 0);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView());

  status_or = Status(345);
  EXPECT_FALSE(status_or.ok());
  EXPECT_FALSE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), 345);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView(""));

  status_or = StringView();
  EXPECT_TRUE(status_or.ok());
  EXPECT_EQ(status_or.value(), StringView(""));
  EXPECT_TRUE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), 0);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView());

  status_or = ReturnStatusWithMessage<StringView>(
      90210, ProgmemStringView("Beverly Hills"));
  EXPECT_FALSE(status_or.ok());
  EXPECT_FALSE(status_or.status().ok());
  EXPECT_EQ(status_or.status().code(), 90210);
  EXPECT_EQ(status_or.status().message(), ProgmemStringView("Beverly Hills"));
}

TEST(StatusOrTest, Ctors) {
  {
    StatusOr<int> status_or_int(1);
    EXPECT_TRUE(status_or_int.ok());
    EXPECT_EQ(status_or_int.status(), OkStatus());
    EXPECT_EQ(status_or_int.value(), 1);
  }
  {
    StatusOr<int> status_or_int(OkStatus());
    EXPECT_TRUE(status_or_int.ok());
    EXPECT_EQ(status_or_int.status(), OkStatus());
    EXPECT_EQ(status_or_int.value(), 0);  // Default value for an int;
  }
  {
    StatusOr<int> status_or_int(StatusOr<int>(345));
    EXPECT_TRUE(status_or_int.ok());
    EXPECT_EQ(status_or_int.status(), OkStatus());
    EXPECT_EQ(status_or_int.value(), 345);
  }
  {
    StatusOr<int> status_or_int(Status(123));
    EXPECT_FALSE(status_or_int.ok());
    EXPECT_EQ(status_or_int.status(), Status(123));
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
    StatusOr<int> status_or_value1 = Status(123);
    StatusOr<int> status_or_value2(0);
    did_return = true;
    MCU_ASSIGN_OR_RETURN(status_or_value2, status_or_value1);
    did_return = false;
    ADD_FAILURE() << "Should have returned! status_or_value2="
                  << status_or_value2;
    return Status(3232);
  };
  EXPECT_EQ(outer(), Status(123));
  EXPECT_TRUE(did_return);
}

TEST(StatusOrTest, AssignOrReturn_CallReturnsError) {
  auto inner = []() -> StatusOr<int> { return Status(123); };
  bool did_return = false;
  auto outer = [&]() -> Status {
    did_return = true;
    MCU_ASSIGN_OR_RETURN(auto value_in_status_or, inner());
    did_return = false;
    ADD_FAILURE() << "Should have returned! value_in_status_or="
                  << value_in_status_or;
    return Status(3232);
  };
  EXPECT_EQ(outer(), Status(123));
  EXPECT_TRUE(did_return);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
