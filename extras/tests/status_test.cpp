#include "status.h"

#include <stdint.h>

#include <string>
#include <tuple>
#include <utility>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/progmem_string_view_utils.h"
#include "extras/test_tools/status_test_utils.h"
#include "gtest/gtest.h"
#include "progmem_string_data.h"
#include "progmem_string_view.h"
#include "status_code.h"

namespace mcucore {
namespace test {
namespace {

using ::testing::Not;
using ::testing::StartsWith;

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
  EXPECT_EQ(PrintStatus(status), absl::StrCat("{.code=InvalidArgument}"));
}

TEST(StatusTest, LocalNotOkWithMessage) {
  Status status(StatusCode::kNotFound, ProgmemStringView("Bar"));
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kNotFound);
  EXPECT_EQ(status.message(), ProgmemStringView("Bar"));
  EXPECT_EQ(PrintStatus(status), "{.code=NotFound, .message=\"Bar\"}");
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
  EXPECT_EQ(PrintStatus(status), "{.code=NotFound}");
}

TEST(StatusTest, ReturnedNotOkWithMessage) {
  auto status = ReturnStatusWithMessage(
      StatusCode::kResourceExhausted, ProgmemStringView("\010foo\015\012bar"));
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), StatusCode::kResourceExhausted);
  EXPECT_EQ(MakeStdString(status.message()), "\bfoo\r\nbar");
  EXPECT_EQ(status.message(), ProgmemStringView("\bfoo\r\nbar"));
  EXPECT_EQ(PrintStatus(status),
            R"({.code=ResourceExhausted, .message="\x08\x66oo\r\nbar"})");
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

TEST(StatusTest, NamedErrorFunctions) {
  using MakeNamedErrorFn = Status (*)(ProgmemStringView);
  using IsNamedErrorFn = bool (*)(const Status&);
  using NamedErrorInfo =
      std::tuple<StatusCode, MakeNamedErrorFn, IsNamedErrorFn>;

  std::vector<NamedErrorInfo> named_error_infos = {
      std::make_tuple(StatusCode::kDataLoss, DataLossError, IsDataLoss),
      std::make_tuple(StatusCode::kFailedPrecondition, FailedPreconditionError,
                      IsFailedPrecondition),
      std::make_tuple(StatusCode::kInternal, InternalError, IsInternal),
      std::make_tuple(StatusCode::kInvalidArgument, InvalidArgumentError,
                      IsInvalidArgument),
      std::make_tuple(StatusCode::kNotFound, NotFoundError, IsNotFound),
      std::make_tuple(StatusCode::kOutOfRange, OutOfRangeError, IsOutOfRange),
      std::make_tuple(StatusCode::kResourceExhausted, ResourceExhaustedError,
                      IsResourceExhausted),
      std::make_tuple(StatusCode::kUnimplemented, UnimplementedError,
                      IsUnimplemented),
      std::make_tuple(StatusCode::kUnknown, UnknownError, IsUnknown)};

  for (int i = 0; i < named_error_infos.size(); ++i) {
    const auto code = std::get<0>(named_error_infos[i]);
    const auto make_error = std::get<1>(named_error_infos[i]);
    Status status = make_error(MCU_PSV("Foo"));

    EXPECT_FALSE(status.ok());
    EXPECT_EQ(status.code(), code);
    EXPECT_THAT(status, Not(IsOk()));
    EXPECT_THAT(status, StatusIs(code));
    EXPECT_THAT(status, StatusIs(code, "Foo"));
    EXPECT_THAT(status, StatusIs(code, StartsWith("Fo")));

    for (int j = 0; j < named_error_infos.size(); ++j) {
      const auto is_error = std::get<2>(named_error_infos[j]);
      if (i == j) {
        EXPECT_TRUE(is_error(status));
        EXPECT_THAT(status, StatusIs(std::get<0>(named_error_infos[j])));
        EXPECT_THAT(status, StatusIs(std::get<0>(named_error_infos[j]), "Foo"));
      } else {
        EXPECT_FALSE(is_error(status));
        EXPECT_THAT(status, Not(StatusIs(std::get<0>(named_error_infos[j]))));
        EXPECT_THAT(status,
                    Not(StatusIs(std::get<0>(named_error_infos[j]), "Foo")));
      }
    }
  }
}

TEST(StatusTest, ReturnIfErrorWithError) {
  auto do_return = [](Status to_be_checked) {
    EXPECT_THAT(to_be_checked, Not(IsOk()));
    MCU_RETURN_IF_ERROR(to_be_checked);
    ADD_FAILURE() << "Should not have reached here";
    return OkStatus();
  };
  EXPECT_TRUE(IsDataLoss(do_return(DataLossError())));
}

TEST(StatusTest, ReturnIfErrorWithOk) {
  bool reached_endpoint = false;
  auto do_not_return = [&reached_endpoint](Status to_be_checked) {
    EXPECT_THAT(to_be_checked, IsOk());
    EXPECT_FALSE(reached_endpoint);
    MCU_RETURN_IF_ERROR(to_be_checked);
    reached_endpoint = true;
    return NotFoundError();
  };
  EXPECT_TRUE(IsNotFound(do_not_return(OkStatus())));
  EXPECT_TRUE(reached_endpoint);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
