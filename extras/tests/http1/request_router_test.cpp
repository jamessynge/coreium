#include "http1/request_router.h"

// TODO(jamessynge): Trim down the includes after writing tests.
#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/print_value_to_std_string.h"
#include "extras/test_tools/sample_printable.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "http1/request_decoder.h"
#include "strings/progmem_string_data.h"

namespace mcucore {
namespace http1 {
namespace test {
namespace {

// TODO(jamessynge): Trim down the using declarations after writing tests.
using ::mcucore::PrintValueToStdString;
using ::mcucore::http1::RequestDecoder;
using ::mcucore::http1::RequestDecoderListener;
using ::mcucore::http1::RequestRouter;
using ::mcucore::test::PrintToStdString;
using ::mcucore::test::SamplePrintable;
using ::testing::AllOf;
using ::testing::AnyNumber;
using ::testing::AnyOf;
using ::testing::Contains;
using ::testing::ContainsRegex;
using ::testing::ElementsAre;
using ::testing::EndsWith;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::InSequence;
using ::testing::IsEmpty;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Not;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SizeIs;
using ::testing::StartsWith;
using ::testing::StrictMock;

// using RootRouter = RequestRouter < RequestRouterEntry < MCU_PSD();

TEST(NoFixture_RequestRouterTest, NoFixtureTest) {
  // TODO(jamessynge): Describe if not really obvious.
  EXPECT_EQ(1, 1);
}

class RequestRouterTest : public testing::Test {
 protected:
  RequestRouterTest() {}
  void SetUp() override {}
};

TEST_F(RequestRouterTest, FixturedTest) {
  // TODO(jamessynge): Describe if not really obvious.
  EXPECT_EQ(1, 1);
}

}  // namespace
}  // namespace test
}  // namespace http1
}  // namespace mcucore
