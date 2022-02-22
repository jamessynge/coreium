#ifndef MCUCORE_EXTRAS_TEST_TOOLS_MOCK_STREAM_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_MOCK_STREAM_H_

#include "extras/host/arduino/stream.h"
#include "extras/test_tools/mock_print.h"
#include "gmock/gmock.h"

namespace test {

class MockStream : public Stream, public MockPrint {
 public:
  MOCK_METHOD(int, available, (), (override));
  MOCK_METHOD(int, read, (), (override));
  MOCK_METHOD(int, peek, (), (override));
};

}  // namespace test

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_MOCK_STREAM_H_
