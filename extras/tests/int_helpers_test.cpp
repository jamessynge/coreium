#include "int_helpers.h"

#include <iostream>
#include <limits>
#include <ostream>

#include "gtest/gtest.h"
#include "semistd/type_traits.h"

namespace mcucore {
namespace test {
namespace {

template <typename T, typename U>
void VerifyConversion(const T value, const U expected_unsigned_value) {
  EXPECT_EQ(sizeof(T), sizeof(U));
  const auto actual_unsigned_value = ToUnsigned(value);
  EXPECT_TRUE((is_same<decltype(actual_unsigned_value), const U>::value));
  EXPECT_EQ(ToUnsigned(value), expected_unsigned_value);
}

template <typename T, typename U>
void VerifyToUnsigned(const T value, const U expected_unsigned_value) {
  EXPECT_EQ(sizeof(T), sizeof(U));
  const auto actual_unsigned_value = ToUnsigned(value);
  EXPECT_TRUE((is_same<decltype(actual_unsigned_value), const U>::value));
  EXPECT_EQ(ToUnsigned(value), expected_unsigned_value);
}

template <typename T, typename U>
void VerifySignedToUnsigned() {
  VerifyToUnsigned<T, U>(std::numeric_limits<T>::min(),
                         std::numeric_limits<U>::max() / 2 + 1);
  VerifyToUnsigned<T, U>(-1, std::numeric_limits<U>::max());
  VerifyToUnsigned<T, U>(0, 0);
  VerifyToUnsigned<T, U>(std::numeric_limits<T>::max(),
                         std::numeric_limits<U>::max() / 2);
}

template <typename T, typename U,
          enable_if_t<sizeof(T) == sizeof(U), bool> = true>
bool MaybeVerifySignedToUnsigned() {
  VerifySignedToUnsigned<T, U>();
  return true;
}

template <typename T, typename U,
          enable_if_t<sizeof(T) != sizeof(U), bool> = false>
bool MaybeVerifySignedToUnsigned() {
  // Sizes don't match (e.g. long is not 4 bytes on this platform).
  return false;
}

template <typename T, typename U>
void VerifyUnsignedToUnsigned() {
  VerifyToUnsigned<T, U>(0, 0);
  VerifyToUnsigned<T, U>(std::numeric_limits<T>::max() / 2,
                         std::numeric_limits<U>::max() / 2);
  VerifyToUnsigned<T, U>(std::numeric_limits<T>::max() / 2 + 1,
                         std::numeric_limits<U>::max() / 2 + 1);
  VerifyToUnsigned<T, U>(std::numeric_limits<T>::max(),
                         std::numeric_limits<U>::max());
}

template <typename T, typename U,
          enable_if_t<sizeof(T) == sizeof(U), bool> = true>
bool MaybeVerifyUnsignedToUnsigned() {
  VerifyUnsignedToUnsigned<T, U>();
  return true;
}

template <typename T, typename U,
          enable_if_t<sizeof(T) != sizeof(U), bool> = false>
bool MaybeVerifyUnsignedToUnsigned() {
  // Sizes don't match (e.g. long is not 4 bytes on this platform).
  return false;
}

TEST(IntHelpersTest, Int8) {
  VerifySignedToUnsigned<int8_t, uint8_t>();
  VerifySignedToUnsigned<signed char, uint8_t>();

  VerifyConversion<char, uint8_t>(0, 0);
  VerifyConversion<char, uint8_t>(127, 127);

  VerifyUnsignedToUnsigned<uint8_t, uint8_t>();
  VerifyUnsignedToUnsigned<unsigned char, uint8_t>();
}

TEST(IntHelpersTest, Int16) {
  VerifySignedToUnsigned<int16_t, uint16_t>();
  VerifyUnsignedToUnsigned<uint16_t, uint16_t>();
}

TEST(IntHelpersTest, Int32) {
  VerifySignedToUnsigned<int32_t, uint32_t>();
  VerifyUnsignedToUnsigned<uint32_t, uint32_t>();
}

TEST(IntHelpersTest, Int64) {
  VerifySignedToUnsigned<int64_t, uint64_t>();
  VerifyUnsignedToUnsigned<uint64_t, uint64_t>();
}

TEST(IntHelpersTest, Short) {
  using Signed = signed short int;      // NOLINT
  using Unsigned = unsigned short int;  // NOLINT

  const bool type_is_2bytes = MaybeVerifySignedToUnsigned<Signed, uint16_t>();
  EXPECT_EQ(type_is_2bytes,
            (MaybeVerifyUnsignedToUnsigned<Unsigned, uint16_t>()));

  const bool type_is_4bytes = MaybeVerifySignedToUnsigned<Signed, uint32_t>();
  EXPECT_EQ(type_is_4bytes,
            (MaybeVerifyUnsignedToUnsigned<Unsigned, uint32_t>()));

  EXPECT_NE(type_is_2bytes, type_is_4bytes);
}

TEST(IntHelpersTest, Int) {
  // There's no support here for the ILP64 data model, where int is 64-bits.

  using Signed = signed int;      // NOLINT
  using Unsigned = unsigned int;  // NOLINT

  const bool type_is_2bytes = MaybeVerifySignedToUnsigned<Signed, uint16_t>();
  EXPECT_EQ(type_is_2bytes,
            (MaybeVerifyUnsignedToUnsigned<Unsigned, uint16_t>()));

  const bool type_is_4bytes = MaybeVerifySignedToUnsigned<Signed, uint32_t>();
  EXPECT_EQ(type_is_4bytes,
            (MaybeVerifyUnsignedToUnsigned<Unsigned, uint32_t>()));

  EXPECT_NE(type_is_2bytes, type_is_4bytes);
}

TEST(IntHelpersTest, Long) {
  using Signed = signed long int;      // NOLINT
  using Unsigned = unsigned long int;  // NOLINT

  const bool type_is_4bytes = MaybeVerifySignedToUnsigned<Signed, uint32_t>();
  EXPECT_EQ(type_is_4bytes,
            (MaybeVerifyUnsignedToUnsigned<Unsigned, uint32_t>()));

  const bool type_is_8bytes = MaybeVerifySignedToUnsigned<Signed, uint64_t>();
  EXPECT_EQ(type_is_8bytes,
            (MaybeVerifyUnsignedToUnsigned<Unsigned, uint64_t>()));

  EXPECT_NE(type_is_4bytes, type_is_8bytes);
}

}  // namespace
}  // namespace test
}  // namespace mcucore
