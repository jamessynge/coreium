#ifndef MCUCORE_EXTRAS_FUTURES_HTTP1_QUERY_STRING_DECODER_H_
#define MCUCORE_EXTRAS_FUTURES_HTTP1_QUERY_STRING_DECODER_H_

// QueryStringDecoder is (TO BE) a decoder of the query string of the path
// portion of the first line of an HTTP/1.1 Request Message (but not body)
// for tiny web servers, i.e. those intended to run on a very low-memory
// system, such as on a microcontroller. The aim is to be able to decode most
// HTTP/1.1 requests, though with the limitation that it can't interpret
// 'tokens' that are longer than the maximum size buffer that the system can
// support (i.e. if the name of a header is longer than that limit, the listener
// will be informed of the issue, and the header name and value will be skipped
// over).
//
// This originated as RequestDecoder in TinyAlpacaServer, which was targeted at
// a very specific set of URL paths and header names.
//
// NOTE: The syntax for the query portion of a URI is not as clearly specified
// as the rest of HTTP (AFAICT), so I'm assuming that:
//
// 1) A name is composed of upper and/or lower case ASCII letters (because those
//    are used by ASCOM Alpaca);
// 2) A value is any non-control character value, excluding space.
// 3) A name is followed by '=' and then by a value.
// 4) A name may not be empty, but some values may be empty; for example,
//    ClientID must be specified in order for this decoder to translate from
//    string to int, but some other parameters have their semantics provided by
//    the calling program, so we defer validation to that calling program
// 5) The HTTP client will not send percent encoded characters; these are are
//    not detected and decoded by this decoder, and are just treated as regular
//    characters. If they're in values that must be explicitly matched or
//    decoded by this decoder, that operation is likely to fail.

namespace mcucore {

class QueryStringDecoder {
 public:
  QueryStringDecoder();
  virtual ~QueryStringDecoder();

  // TODO(jamessynge): Define and document methods.

 protected:
 private:
  // TODO(jamessynge): Define and document private methods and fields.
};

}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_FUTURES_HTTP1_QUERY_STRING_DECODER_H_
