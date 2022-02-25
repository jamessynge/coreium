#ifndef MCUCORE_EXTRAS_TEST_TOOLS_MOCK_PRINT_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_MOCK_PRINT_H_

#include "extras/host/arduino/print.h"
#include "gmock/gmock.h"

// Even though Print is in the root namespace, I've chosen to place MockPrint
// into mcucore::test.
namespace mcucore {
namespace test {

class MockPrint : public Print {
 public:
  MOCK_METHOD(size_t, write, (uint8_t), (override));
  MOCK_METHOD(size_t, write, (const uint8_t *, size_t), (override));
  MOCK_METHOD(void, flush, (), (override));
};

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_MOCK_PRINT_H_
