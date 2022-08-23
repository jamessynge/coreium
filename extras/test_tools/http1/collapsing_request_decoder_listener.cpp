#include "extras/test_tools/http1/collapsing_request_decoder_listener.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "extras/test_tools/http1/string_utils.h"
#include "extras/test_tools/string_view_utils.h"
#include "gtest/gtest.h"
#include "http1/request_decoder.h"
#include "http1/request_decoder_constants.h"
#include "strings/string_view.h"

namespace mcucore {
namespace http1 {
namespace test {

using ::mcucore::test::MakeStdStringView;
using ::mcucore::test::MakeStringView;
using ::mcucore::test::SplitEveryN;

void CollapsingRequestDecoderListener::OnEvent(const OnEventData& data) {
  if (IsNotAccumulating(data)) {
    rdl_.OnEvent(data);
  }
}

void CollapsingRequestDecoderListener::OnCompleteText(
    const OnCompleteTextData& data) {
  // When we get the complete text of the entity, then exactly that text should
  // be in the input buffer.
  EXPECT_TRUE(data.GetFullDecoderInput().contains(data.text));
  if (IsNotAccumulating(data)) {
    rdl_.OnCompleteText(data);
  }
}

void CollapsingRequestDecoderListener::OnPartialText(
    const OnPartialTextData& data) {
  if (data.token == EPartialToken::kRawQueryString) {
    if (IsNotAccumulating(data)) {
      rdl_.OnPartialText(data);
    }
    return;
  }

  if (data.position == EPartialTokenPosition::kFirst) {
    if (!IsNotAccumulating(data)) {
      // Oops, should NOT already accumulating something else.
      return;
    }
    token_ = data.token;
    text_ = MakeStdStringView(data.text);
    VLOG(1) << "Got first part of a " << token_.value();
    return;
  }

  if (!token_.has_value()) {
    // Oops, SHOULD already be accumulating a token.
    ADD_FAILURE() << "Expected to be accumulating data for a " << data.token;
    return;
  }

  if (token_.value() != data.token) {
    // Oops, SHOULD already accumulating something else.
    ADD_FAILURE() << "Expected to be accumulating data for a " << token_.value()
                  << ", not a " << data.token;
    return;
  }
  VLOG(1) << "Got " << data.position << " part of a " << data.token;

  text_ += MakeStdStringView(data.text);
  if (data.position != EPartialTokenPosition::kLast) {
    return;
  }

  constexpr auto kMaxSize = StringView::kMaxSize;
  if (text_.size() <= kMaxSize) {
    OnCompleteTextData complete_data(data);
    switch (data.token) {
#define PARTIAL_TO_FULL_TOKEN(NAME)     \
  case EPartialToken::NAME:             \
    complete_data.token = EToken::NAME; \
    break;

      PARTIAL_TO_FULL_TOKEN(kPathSegment);
      PARTIAL_TO_FULL_TOKEN(kParamName);
      PARTIAL_TO_FULL_TOKEN(kParamValue);
      PARTIAL_TO_FULL_TOKEN(kHeaderName);
      PARTIAL_TO_FULL_TOKEN(kHeaderValue);

      case EPartialToken::kRawQueryString:
        ADD_FAILURE() << "Unexpectedly accumulating data for a kRawQueryString";
        return;
    }
    std::string copy = text_;
    text_.clear();
    complete_data.text = MakeStringView(copy);
    token_.reset();
    rdl_.OnCompleteText(complete_data);
    return;
  }

  // Too large to pass via OnCompleteText.
  OnPartialTextData partial_data = data;
  partial_data.position = EPartialTokenPosition::kFirst;
  auto parts = SplitEveryN(text_, kMaxSize);
  text_.clear();
  token_.reset();
  for (const auto& part : parts) {
    partial_data.text = MakeStringView(part);
    rdl_.OnPartialText(partial_data);
    partial_data.position = EPartialTokenPosition::kMiddle;
  }
  partial_data.position = EPartialTokenPosition::kLast;
  partial_data.text = StringView();
  rdl_.OnPartialText(partial_data);
}

void CollapsingRequestDecoderListener::OnError(const OnErrorData& data) {
  if (IsNotAccumulating(data)) {
    rdl_.OnError(data);
  }
}

template <typename T>
bool CollapsingRequestDecoderListener::IsNotAccumulating(const T& data) {
  if (token_.has_value()) {
    ADD_FAILURE() << "Expected to not be accumulating data for a "
                  << token_.value();
    token_.reset();
    data.StopDecoding();
    return false;
  }
  return true;
}

}  // namespace test
}  // namespace http1
}  // namespace mcucore
