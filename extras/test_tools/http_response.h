#ifndef MCUCORE_EXTRAS_TEST_TOOLS_HTTP_RESPONSE_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_HTTP_RESPONSE_H_

// Provides a trivial parser of HTTP/1 responses. There is no validation that
// the strings make sense, only that the delimiters are in the right places,
// and that the status_code is a non-negative integer.
//
// Author: james.synge@gmail.com

#include <stddef.h>

#include <map>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "extras/test_tools/case_insensitive_less.h"
#include "extras/test_tools/json_decoder.h"

namespace mcucore {
namespace test {

struct HttpResponse {
  // Returns an HttpResponse based on the provided string, if it can be parsed
  // as such, else returns an error.
  static absl::StatusOr<HttpResponse> Make(std::string response);

  // Returns OK if is an OK response, else returns an error that explains what
  // it actually is.
  absl::Status IsOk() const;

  std::vector<std::string> GetHeaderValues(const std::string& name) const;
  absl::StatusOr<std::string> GetSoleHeaderValue(const std::string& name) const;
  bool HasHeader(const std::string& name) const;
  bool HasHeaderValue(const std::string& name, const std::string& value) const;

  // Decodes the Content-Length header as a size_t. Returns a NotFoundError if
  // there isn't a Content-Length header; returns an InvalidArgumentError if the
  // value of the Content-Length header can't be decoded as a size_t.
  absl::StatusOr<size_t> GetContentLength() const;

  // Returns the Content-Type header's value, if a single such header is
  // present. Returns NotFoundError if there isn't a Content-Type header.
  absl::StatusOr<std::string> GetContentType() const;

  std::string http_version;
  int status_code;
  std::string status_message;
  std::multimap<std::string, std::string, mcucore::test::CaseInsensitiveLess>
      headers;

  // Everything after the headers, whether it is exactly the required size for
  // the body, or more or less for some reason.
  std::string body_and_beyond;

  // If the Content-Type is application/json, and the body_and_beyond.size() is
  // at least Content-Length, and that decodes as JSON, then the decoded
  // JsonValue is stored here, and the Content-Length is removed from
  // body_and_beyond. If the body should be decoded as JSON, but is ill-formed,
  // then Make (above) will return an error.
  JsonValue json_value;
};

// A helper for creating a response in tests of working with HTTP responses
// (initially in tests of AlpacaResponseValidator).
absl::StatusOr<std::string> AssembleHttpResponseMessage(
    int status_code,
    const std::vector<std::pair<std::string, std::string>>& headers,
    const std::string& body = "", bool add_content_length_header = true);

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_HTTP_RESPONSE_H_
