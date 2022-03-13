#include "status.h"

#include <stdint.h>

#include <string>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/progmem_string_view_utils.h"
#include "gtest/gtest.h"
#include "progmem_string_view.h"
#include "status_code.h"

namespace mcucore {
namespace test {
namespace {

Status ReturnStatusWithoutMessage(StatusCode code) { return Status(code); }
Status ReturnStatusWithMessage(StatusCode code, ProgmemStringView message) {
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
  Status status(StatusCode::kOk);
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kOk);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status), "OK");
}

TEST(StatusTest, LocalOkWithMessage) {
  Status status(StatusCode::kOk, ProgmemStringView("Foo"));
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kOk);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status), "OK");
}

TEST(StatusTest, LocalNotOk) {
  Status status(StatusCode::kInvalidArgument);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kInvalidArgument);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status),
            absl::StrCat("{.code=", StatusCode::kInvalidArgument, "}"));
}

TEST(StatusTest, LocalNotOkWithMessage) {
  Status status(StatusCode::kNotFound, ProgmemStringView("Bar"));
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kNotFound);
  EXPECT_EQ(status.message(), ProgmemStringView("Bar"));
  EXPECT_EQ(PrintStatus(status), absl::StrCat("{.code=", StatusCode::kNotFound,
                                              R"(, .message="Bar"})"));
}

TEST(StatusTest, ReturnedOk) {
  auto status = ReturnStatusWithoutMessage(StatusCode::kOk);
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kOk);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status), "OK");
}

TEST(StatusTest, ReturnedNotOk) {
  auto status = ReturnStatusWithoutMessage(StatusCode::kNotFound);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kNotFound);
  EXPECT_EQ(status.message(), ProgmemStringView());
  EXPECT_EQ(PrintStatus(status),
            absl::StrCat("{.code=", StatusCode::kNotFound, "}"));
}

TEST(StatusTest, ReturnedNotOkWithMessage) {
  auto status = ReturnStatusWithMessage(
      StatusCode::kResourceExhausted, ProgmemStringView("\010foo\015\012bar"));
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kResourceExhausted);
  EXPECT_EQ(MakeStdString(status.message()), "\bfoo\r\nbar");
  EXPECT_EQ(status.message(), ProgmemStringView("\bfoo\r\nbar"));
  EXPECT_EQ(PrintStatus(status),
            absl::StrCat("{.code=", StatusCode::kResourceExhausted,
                         R"(, .message="\x08\x66oo\r\nbar"})"));
}

enum Convertable { kConvertable = 123456 };
StatusCode ToStatusCode(Convertable v) { return static_cast<StatusCode>(v); }

TEST(StatusTest, Convertable_HasToStatusCode) {
  EXPECT_TRUE(has_to_status_code<Convertable>::value);
}

// Enum NotConvertable has a ToStatusCode, but it doesn't return the correct
// type, i.e. StatusCode.
enum NotConvertable { kNotConvertable };
NotConvertable ToStatusCode(NotConvertable v) { return v; }

TEST(StatusTest, NotConvertable_Not_HasToStatusCode) {
  EXPECT_EQ(kNotConvertable, ToStatusCode(kNotConvertable));
  EXPECT_FALSE(has_to_status_code<NotConvertable>::value);
}

// float has a ToStatusCode, but it isn't an enum.
StatusCode ToStatusCode(float v) { return StatusCode::kInvalidArgument; }

TEST(StatusTest, Float_Not_HasToStatusCode) {
  EXPECT_EQ(ToStatusCode(0.0F), StatusCode::kInvalidArgument);
  EXPECT_FALSE(has_to_status_code<float>::value);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
