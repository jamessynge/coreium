#include "status_or.h"

#include <stdint.h>

#include "gtest/gtest.h"
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

}  // namespace
}  // namespace test
}  // namespace mcucore
