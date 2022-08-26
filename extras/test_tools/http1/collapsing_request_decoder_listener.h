#ifndef MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_COLLAPSING_REQUEST_DECODER_LISTENER_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_COLLAPSING_REQUEST_DECODER_LISTENER_H_

// Accumulates the text provided in multiple OnPartialText calls for the same
// entity (a path segment, a header name or value) into a single string and
// calls OnCompleteText once the last part of the string is provided to the
// listener. This makes it easier to write expectations.
//
// Note that if the total length of the accumulated string is greater than an
// StringView can represent, the accumulated string will instead be
// passed to OnPartialText in large pieces.
//
// Author: james.synge@gmail.com

#include <optional>
#include <string>

#include "http1/request_decoder.h"
#include "http1/request_decoder_constants.h"

namespace mcucore {
namespace http1 {
namespace test {

class CollapsingRequestDecoderListener : public RequestDecoderListener {
 public:
  explicit CollapsingRequestDecoderListener(RequestDecoderListener& rdl)
      : rdl_(rdl) {}

  void OnEvent(const OnEventData& data) override;
  void OnCompleteText(const OnCompleteTextData& data) override;
  void OnPartialText(const OnPartialTextData& data) override;
  void OnError(const OnErrorData& data) override;

 private:
  template <typename T>
  bool IsNotAccumulating(const T& data);

  RequestDecoderListener& rdl_;
  std::optional<EPartialToken> token_;
  std::string text_;
};

}  // namespace test
}  // namespace http1
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_HTTP1_COLLAPSING_REQUEST_DECODER_LISTENER_H_
