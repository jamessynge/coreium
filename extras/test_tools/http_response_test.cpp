#include "extras/test_tools/http_response.h"

#include <type_traits>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "extras/test_tools/json_decoder.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/task/status_macros.h"

namespace mcucore {
namespace test {
namespace {

using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;
using ::testing::status::IsOkAndHolds;
using ::testing::status::StatusIs;

TEST(HttpResponseTest, MinimalInput) {
  ASSERT_OK_AND_ASSIGN(auto hr, HttpResponse::Make("V 0\r\n\r\n"));
  EXPECT_EQ(hr.http_version, "V");
  EXPECT_EQ(hr.status_code, 0);
  EXPECT_EQ(hr.status_message, "");
  EXPECT_THAT(hr.headers, IsEmpty());
  EXPECT_EQ(hr.body_and_beyond, "");
  EXPECT_THAT(hr.IsOk(), StatusIs(absl::StatusCode::kFailedPrecondition,
                                  HasSubstr("V 0 ")));
}

TEST(HttpResponseTest, HasStatusMessageAndBody) {
  ASSERT_OK_AND_ASSIGN(
      auto hr,
      HttpResponse::Make("HTTP/1.1 200 OK, really\r\n\r\n\r\n\r\nFoo\r\n\r\n"));
  EXPECT_EQ(hr.http_version, "HTTP/1.1");
  EXPECT_EQ(hr.status_code, 200);
  EXPECT_EQ(hr.status_message, "OK, really");
  EXPECT_THAT(hr.IsOk(), StatusIs(absl::StatusCode::kFailedPrecondition,
                                  HasSubstr("HTTP/1.1 200 OK, really")));
  EXPECT_THAT(hr.headers, IsEmpty());
  EXPECT_EQ(hr.body_and_beyond, "\r\n\r\nFoo\r\n\r\n");
}

TEST(HttpResponseTest, Full) {
  ASSERT_OK_AND_ASSIGN(auto hr, HttpResponse::Make("HTTP/1.1 200 OK\r\n"
                                                   "Host: example.com\r\n"
                                                   "Content-Type:text/plain\r\n"
                                                   "Set-Cookie:  a = b\r\n"
                                                   "Set-Cookie:\t  c=d \t\r\n"
                                                   "Empty:\t \t\r\n"
                                                   "\r\n"
                                                   "Some body."));
  EXPECT_EQ(hr.http_version, "HTTP/1.1");
  EXPECT_EQ(hr.status_code, 200);
  EXPECT_EQ(hr.status_message, "OK");
  EXPECT_OK(hr.IsOk());
  EXPECT_THAT(hr.headers, UnorderedElementsAre(
                              Pair("Host", "example.com"),
                              Pair("Content-Type", "text/plain"),
                              Pair("Set-Cookie", "a = b"),
                              Pair("Set-Cookie", "c=d"), Pair("Empty", "")));
  EXPECT_EQ(hr.body_and_beyond, "Some body.");
  EXPECT_EQ(hr.json_value, JsonValue());  // Unset
  EXPECT_THAT(
      hr.GetSoleHeaderValue("Set-Cookie"),
      StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr("More than one")));
  EXPECT_THAT(hr.GetSoleHeaderValue("FooBar"),
              StatusIs(absl::StatusCode::kNotFound));
}

TEST(HttpResponseTest, IncompleteOrInvalidHeaders) {
  EXPECT_THAT(
      HttpResponse::Make(""),
      StatusIs(absl::StatusCode::kInvalidArgument, "End of headers not found"));
  EXPECT_THAT(
      HttpResponse::Make("\r\n\r\n"),
      StatusIs(absl::StatusCode::kInvalidArgument, "Headers not found"));
  EXPECT_THAT(HttpResponse::Make("\r\nMissing: Status-Line\r\n\r\n"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Unable to split status line")));
  EXPECT_THAT(HttpResponse::Make("HTTP/1.1\r\nMissing: Status-Code\r\n\r\n"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Unable to split status line")));
  EXPECT_THAT(
      HttpResponse::Make("HTTP/1.1 two-hundred\r\nBad: Status-Code\r\n\r\n"),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("Unable to parse status code as an integer")));
  EXPECT_THAT(HttpResponse::Make("HTTP/1.1 200\r\nNo-Colon\r\n\r\n"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Unable to split header line: No-Colon")));
}

TEST(HttpResponseTest, EmptyJsonObject) {
  ASSERT_OK_AND_ASSIGN(auto hr,
                       HttpResponse::Make("HTTP/1.1 200 OK\r\n"
                                          "Content-Type: application/json \r\n"
                                          "Content-Length: 2\r\n"
                                          "\r\n"
                                          "{}  "));
  EXPECT_EQ(hr.http_version, "HTTP/1.1");
  EXPECT_EQ(hr.status_code, 200);
  EXPECT_EQ(hr.status_message, "OK");
  EXPECT_OK(hr.IsOk());
  EXPECT_THAT(hr.headers,
              UnorderedElementsAre(Pair("Content-Type", "application/json"),
                                   Pair("Content-Length", "2")));
  EXPECT_EQ(hr.body_and_beyond, "  ");
  EXPECT_EQ(hr.json_value, JsonObject());
  EXPECT_TRUE(hr.HasHeader("content-type"));
  EXPECT_FALSE(hr.HasHeader("Host"));
  EXPECT_THAT(hr.GetSoleHeaderValue("content-Length"), IsOkAndHolds("2"));
  EXPECT_THAT(hr.GetContentLength(), IsOkAndHolds(2));
}

TEST(HttpResponseTest, TruncatedJsonResponse) {
  auto status_or = HttpResponse::Make(
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: application/json \r\n"
      "Content-Length: 2\r\n"
      "\r\n"
      "{");

  ASSERT_FALSE(status_or.ok());
  ASSERT_TRUE(absl::IsInvalidArgument(status_or.status()))
      << status_or.status();
}

TEST(HttpResponseTest, CorruptJsonResponse) {
  auto status_or = HttpResponse::Make(
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: application/json \r\n"
      "Content-Length: 2\r\n"
      "\r\n"
      "{]");

  ASSERT_FALSE(status_or.ok());
  ASSERT_TRUE(absl::IsInvalidArgument(status_or.status()))
      << status_or.status();
}

}  // namespace
}  // namespace test
}  // namespace mcucore
