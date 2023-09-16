# [McuCore's](https://github.com/jamessynge/mcucore) HTTP/1.1 Request Header Decoder

**WORK IN PROGRESS**

This folder has provides support for implementing an HTTP/1.1 server on a
microcontroller. In particular, it provides a decoder (~= parser) for HTTP/1.1
request headers (i.e. the
[request-line](https://www.rfc-editor.org/rfc/rfc9112.html#name-request-line)
and the HTTP name-value header lines (aka
[field-lines](https://www.rfc-editor.org/rfc/rfc9112.html#name-field-syntax))
that follow the request-line.

> This is in McuCore rather than McuNet because it doesn't actually require the
> low-level networking features provided by McuNet. **BUT** it would probably
> make more sense for it to be in McuNet, or in a new repo... but too many repos
> makes life a pain.

The RequestDecoder class is built on the assumption that there is only a minimal
amount of memory available, and so performs no buffering of data on its own; in
fact, it has only two pointer fields. The decoder relies on the caller to
provide an input buffer with the data to be decoded, and to provide a
RequestDecoderListener instance. The listener will be called when significant
events occur, such as identifying the request method (e.g. GET or HEAD),
segments of the request path (e.g. "foo" or "bar" in the path "/foo/bar"), etc.
