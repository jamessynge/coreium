#include "extras/test_tools/pretty_type_name.h"

#include <vector>

#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

TEST(PrettyTypeNameTest, Test) {
  EXPECT_EQ(PrettyTypeName<double>(), "double");

  // This might not be portable... std::vector has multiple template parameters,
  // which aren't reflected here. This was tested with clang.
  EXPECT_EQ(PrettyTypeName<std::vector<int>>(), "std::vector<int>");
}

}  // namespace
}  // namespace test
}  // namespace mcucore
