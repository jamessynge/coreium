#ifndef MCUCORE_EXTRAS_TEST_TOOLS_MOCK_STREAM_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_MOCK_STREAM_H_

// Mock class for Stream. While Stream is in the root namespace, I've chosen to
// place the mock class into a test namespace so that it is really clear that it
// is for use in tests; in some cases this avoids the need for a using
// declaration.
//
// The only types used here are those of the class(es) being mocked and those
// used in the methods being mocked. Even though IWYU would call for more
// headers to be included, I've chosen here to limit the includes in headers
// like this to gmock.h and the header(s) of the class(es) being mocked.
//
// Author: james.synge@gmail.com

#include "extras/host/arduino/stream.h"
#include "gmock/gmock.h"

namespace mcucore {
namespace test {

class MockStream : public Stream {
 public:
  // Print methods:
  MOCK_METHOD(size_t, write, (uint8_t), (override));
  MOCK_METHOD(size_t, write, (const uint8_t *, size_t), (override));
  MOCK_METHOD(void, flush, (), (override));

  // Stream methods:
  MOCK_METHOD(int, available, (), (override));
  MOCK_METHOD(int, read, (), (override));
  MOCK_METHOD(int, peek, (), (override));
};

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_MOCK_STREAM_H_
