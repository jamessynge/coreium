#include "status.h"

#include <stdint.h>

#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

Status ReturnStatusWithoutMessage(uint32_t code) { return Status(code); }
Status ReturnStatusWithMessage(uint32_t code, ProgmemStringView message) {
  return Status(code, message);
}

TEST(StatusTest, LocalOk) {
  Status status(0);
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), 0);
  EXPECT_EQ(status.message(), ProgmemStringView());
}

TEST(StatusTest, LocalOkWithMessage) {
  Status status(0, ProgmemStringView("Foo"));
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), 0);
  EXPECT_EQ(status.message(), ProgmemStringView());
}

TEST(StatusTest, LocalNotOk) {
  Status status(123);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), 123);
  EXPECT_EQ(status.message(), ProgmemStringView());
}

TEST(StatusTest, LocalNotOkWithMessage) {
  Status status(123, ProgmemStringView("Bar"));
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), 123);
  EXPECT_EQ(status.message(), ProgmemStringView("Bar"));
}

TEST(StatusTest, ReturnedOk) {
  auto status = ReturnStatusWithoutMessage(0);
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), 0);
  EXPECT_EQ(status.message(), ProgmemStringView());
}

TEST(StatusTest, ReturnedNotOk) {
  auto status = ReturnStatusWithoutMessage(999999999);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), 999999999);
  EXPECT_EQ(status.message(), ProgmemStringView());
}

TEST(StatusTest, ReturnedNotOkWithMessage) {
  auto status = ReturnStatusWithMessage(11111, ProgmemStringView("baz"));
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), 11111);
  EXPECT_EQ(status.message(), ProgmemStringView("baz"));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
