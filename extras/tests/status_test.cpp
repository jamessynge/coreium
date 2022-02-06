#include "status.h"

#include <stdint.h>

#include <string>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/progmem_string_view_utils.h"
#include "gtest/gtest.h"
#include "progmem_string_view.h"

namespace mcucore {
namespace test {
namespace {

Status ReturnStatusWithoutMessage(uint32_t code) { return Status(code); }
Status ReturnStatusWithMessage(uint32_t code, ProgmemStringView message) {
  return Status(code, message);
}

std::string PrintStatus(const Status& status) {
  PrintToStdString p2ss;
  auto size = status.printTo(p2ss);
  auto result = p2ss.str();
  EXPECT_EQ(result.size(), size);
  return result;
}

TEST(StatusTest, LocalOk) {
  Status status(0);
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), 0);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status), "OK");
}

TEST(StatusTest, LocalOkWithMessage) {
  Status status(0, ProgmemStringView("Foo"));
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), 0);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status), "OK");
}

TEST(StatusTest, LocalNotOk) {
  Status status(123);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), 123);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status), "{.code=123}");
}

TEST(StatusTest, LocalNotOkWithMessage) {
  Status status(1234, ProgmemStringView("Bar"));
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), 1234);
  EXPECT_EQ(status.message(), ProgmemStringView("Bar"));
  EXPECT_EQ(PrintStatus(status), R"({.code=1234, .message="Bar"})");
}

TEST(StatusTest, ReturnedOk) {
  auto status = ReturnStatusWithoutMessage(0);
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), 0);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status), "OK");
}

TEST(StatusTest, ReturnedNotOk) {
  auto status = ReturnStatusWithoutMessage(999999999);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), 999999999);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status), "{.code=999999999}");
}

TEST(StatusTest, ReturnedNotOkWithMessage) {
  auto status =
      ReturnStatusWithMessage(11111, ProgmemStringView("\010foo\015\012bar"));
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), 11111);
  EXPECT_EQ(MakeStdString(status.message()), "\bfoo\r\nbar");
  EXPECT_EQ(status.message(), ProgmemStringView("\bfoo\r\nbar"));
  EXPECT_EQ(PrintStatus(status),
            R"({.code=11111, .message="\x08\x66oo\r\nbar"})");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
