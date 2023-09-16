#ifndef MCUCORE_EXTRAS_FUTURES_HTTP1_PERCENT_DECODER_H_
#define MCUCORE_EXTRAS_FUTURES_HTTP1_PERCENT_DECODER_H_

// Supports undoing percent-encoding during decoding that has been applied to
// path segments, or to names and values in a query string.

#include "experimental/users/jamessynge/arduino/mcunet/src/http1/request_decoder.h"
#include "mcucore_platform.h"
#include "string_view.h"
#include "tiny_string.h"

namespace mcunet {
namespace http1 {

class PercentDecoder : public RequestDecoderListener {
 public:
  PercentDecoder();
  virtual ~PercentDecoder();

  // Set the listener to be used when DecodeBuffer is next called.
  void SetListener(RequestDecoderListener& listener);

  // Clear listener_ (set to nullptr). This means that the decoder will drop
  // events on the floor. This could be useful if one wants to discard the
  // remainder of a message header, continuing decoding until EEvent::kComplete
  // is returned by DecodeBuffer.
  void ClearListener();

  // `data.event` has occurred; usually means that some particular fixed text
  // has been matched.
  void OnEvent(const OnEventData& data) override;

  // `data.token` has been matched; it's complete value is in `data.text`.
  void OnCompleteText(const OnCompleteTextData& data) override;

  // Some portion of `data.token` has been matched, with that portion in
  // `data.text`. The text may be empty, so far just for positions kFirst and
  // kLast.
  void OnPartialText(const OnPartialTextData& data) override;

  // An error has occurred, as described in `data.message`.
  void OnError(const OnErrorData& data) override;

 protected:
 private:
  enum State { kIsRaw, kSawPercent, kHaveFirstChar };
  State state_;

  // If state_==kHaveFirstChar, this is the first of the two characters in the
  // URL encoding of some byte.
  char first_char_;
};

// void PercentDecoder(mcucore::);

}  // namespace http1
}  // namespace mcunet

#endif  // MCUCORE_EXTRAS_FUTURES_HTTP1_PERCENT_DECODER_H_
