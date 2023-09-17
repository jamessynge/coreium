#include "semistd/tuple.h"

// TODO(jamessynge): Trim down the includes after writing tests.
#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/sample_printable.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "semistd/type_traits.h"

namespace mcucore {
namespace test {
namespace {

// TODO(jamessynge): Trim down the using declarations after writing tests.
using ::mcucore::test::PrintToStdString;
using ::mcucore::test::SamplePrintable;
using ::testing::AnyNumber;
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

TEST(TupleTest, CallCtor) {
  tuple<char, int, float> v('a', 1, 1.0F);
  using ActualT = decltype(v);
  using ExpectedT = tuple<char, int, float>;
  static_assert(is_same<ActualT, ExpectedT>::value);
}

TEST(TupleTest, MakeTuple) {
  auto v = make_tuple('a', 1, 1.0F);
  using ActualT = decltype(v);
  using ExpectedT = tuple<char, int, float>;
  static_assert(is_same<ActualT, ExpectedT>::value);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
