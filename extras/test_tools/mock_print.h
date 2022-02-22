#ifndef MCUCORE_EXTRAS_TEST_TOOLS_MOCK_PRINT_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_MOCK_PRINT_H_

#include "extras/host/arduino/print.h"
#include "gmock/gmock.h"

namespace test {

class MockPrint : public Print {
 public:
  MOCK_METHOD(size_t, write, (uint8_t), (override));
  MOCK_METHOD(size_t, write, (const uint8_t *, size_t), (override));
};

}  // namespace test

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_MOCK_PRINT_H_
