#include "print/print_to_buffer.h"

#include "extras/test_tools/test_strings.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

void ExpectSame(const PrintToBuffer& a, const PrintToBuffer& b) {
  EXPECT_EQ(a.buffer_size(), b.buffer_size());
  EXPECT_EQ(a.data_size(), b.data_size());
  EXPECT_EQ(a.bytes_written(), b.bytes_written());
  EXPECT_EQ(a.has_buffer_overflow(), b.has_buffer_overflow());
  EXPECT_EQ(a.buffer(), b.buffer());
  EXPECT_EQ(a.chars(), b.chars());
  EXPECT_EQ(a.ToStringView(), b.ToStringView());
}

TEST(PrintToBufferTest, ByteArrayCtors) {
  uint8_t array[128];
  PrintToBuffer a(array, 128);
  PrintToBuffer b(array);
  ExpectSame(a, b);
  PrintToBuffer c(reinterpret_cast<char*>(array), sizeof array);
  ExpectSame(a, c);
}

TEST(PrintToBufferTest, CharArrayCtors) {
  char array[128];
  PrintToBuffer a(array, 128);
  PrintToBuffer b(array);
  ExpectSame(a, b);
  PrintToBuffer c(reinterpret_cast<uint8_t*>(array), sizeof array);
  ExpectSame(a, c);
}

TEST(PrintToBufferTest, TinyStringCtor) {
  TinyString<128> tiny_string;
  PrintToBuffer a(tiny_string);
  PrintToBuffer b(tiny_string.data(), tiny_string.maximum_size());
  ExpectSame(a, b);
}

TEST(PrintToBufferTest, Prints) {
  uint8_t array[16];
  PrintToBuffer p2b(array);

  p2b.print(123);
  p2b.print(' ');
  p2b.print(MCU_FLASHSTR("abc"));
  p2b.print(' ');
  p2b.print("def");

  EXPECT_EQ(p2b.ToStringView(), StringView("123 abc def"));
}

TEST(PrintToBufferTest, HandlesOverflow) {
  uint8_t array[14];
  PrintToBuffer p2b(array);

  p2b.print(TEST_STR_16);

  EXPECT_TRUE(p2b.has_buffer_overflow());
  EXPECT_EQ(p2b.bytes_written(), 16);
  EXPECT_EQ(p2b.data_size(), 14);
  EXPECT_EQ(p2b.ToStringView(), StringView(TEST_STR_14));

  p2b.Reset();

  EXPECT_FALSE(p2b.has_buffer_overflow());
  EXPECT_EQ(p2b.bytes_written(), 0);
  EXPECT_EQ(p2b.data_size(), 0);
  EXPECT_EQ(p2b.ToStringView(), StringView(""));

  p2b.print(TEST_STR_14);

  EXPECT_FALSE(p2b.has_buffer_overflow());
  EXPECT_EQ(p2b.bytes_written(), 14);
  EXPECT_EQ(p2b.data_size(), 14);
  EXPECT_EQ(p2b.ToStringView(), StringView(TEST_STR_14));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
