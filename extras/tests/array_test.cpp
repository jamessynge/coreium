#include "array.h"

#include <stdint.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {
using ::testing::ElementsAre;

constexpr float kOneFloat[] = {1};
constexpr int16_t kThreeShorts[] = {10, 20, 30};

TEST(ArrayTest, Size1Array) {
  Array<float, 1> array{kOneFloat[0]};
  EXPECT_EQ(array.size(), 1);
  EXPECT_THAT(array, ElementsAre(kOneFloat[0]));

  auto array2 = MakeArray<int>(array[0]);
  EXPECT_EQ(array2.size(), 1);
  EXPECT_THAT(array2, ElementsAre(kOneFloat[0]));

  auto array3 = MakeFromArray(kOneFloat);
  EXPECT_EQ(array2.size(), 1);
  EXPECT_THAT(array2, ElementsAre(array3.at(0)));
}

TEST(ArrayTest, Size2Array) {
  Array<double, 2> array{2, 3};
  EXPECT_EQ(array.size(), 2);
  EXPECT_THAT(array, ElementsAre(2.0, 3.0));
}

TEST(ArrayTest, Size3Array) {
  int smaller[] = {1, 3};
  auto array = MakeArray(10, 20, 30);
  int larger[] = {9, 8, 7, 6, 5, 4};
  EXPECT_EQ(array.size(), 3);
  EXPECT_THAT(array, ElementsAre(10, 20, 30));

  array.copy(smaller);
  EXPECT_EQ(array.size(), 3);
  EXPECT_THAT(array, ElementsAre(1, 3, 30));

  array.copy(larger);
  EXPECT_EQ(array.size(), 3);
  EXPECT_THAT(array, ElementsAre(9, 8, 7));

  // Make sure that adjacent elements have not been overwritten.
  EXPECT_THAT(smaller, ElementsAre(1, 3));
  EXPECT_THAT(larger, ElementsAre(9, 8, 7, 6, 5, 4));
}

TEST(ArrayTest, Size9Array) {
  int smaller[] = {1, 1, 2, 3};
  auto array = MakeArray(10, 20, 30, 40, 50, 60, 70, 80, 90);
  int larger[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  EXPECT_EQ(array.size(), 9);
  EXPECT_THAT(array, ElementsAre(10, 20, 30, 40, 50, 60, 70, 80, 90));

  array.copy(smaller);
  EXPECT_EQ(array.size(), 9);
  EXPECT_THAT(array, ElementsAre(1, 1, 2, 3, 50, 60, 70, 80, 90));

  array.copy(larger);
  EXPECT_EQ(array.size(), 9);
  EXPECT_THAT(array, ElementsAre(10, 9, 8, 7, 6, 5, 4, 3, 2));

  // Make sure that adjacent elements have not been overwritten.
  EXPECT_THAT(smaller, ElementsAre(1, 1, 2, 3));
  EXPECT_THAT(larger, ElementsAre(10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
