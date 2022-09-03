#include "print/print_to_buffer.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

#include "absl/strings/str_cat.h"
#include "extras/test_tools/string_view_utils.h"
#include "extras/test_tools/test_strings.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "strings/string_view.h"

namespace mcucore {
namespace test {
namespace {

using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::StartsWith;

void ExpectIsFull(const PrintToBuffer& p2b) {
  EXPECT_EQ(p2b.data_size(), p2b.buffer_size());
  EXPECT_FALSE(p2b.HasWriteError());
  EXPECT_EQ(p2b.availableForWrite(), 0);
}

void ExpectHasOverflowed(const PrintToBuffer& p2b) {
  EXPECT_TRUE(p2b.HasWriteError());
  EXPECT_EQ(p2b.availableForWrite(), 0);
}

void ExpectIsEmpty(const PrintToBuffer& p2b) {
  EXPECT_EQ(p2b.data_size(), 0);
  EXPECT_FALSE(p2b.HasWriteError());
  EXPECT_EQ(p2b.getWriteError(), 0);
  EXPECT_EQ(p2b.availableForWrite(), p2b.buffer_size());
}

void ExpectSame(const PrintToBuffer& a, const PrintToBuffer& b) {
  EXPECT_EQ(a.buffer_size(), b.buffer_size());
  EXPECT_EQ(a.data_size(), b.data_size());
  EXPECT_EQ(a.HasWriteError(), b.HasWriteError());
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

TEST(PrintToBufferTest, WriteOneByteAtATime) {
  constexpr size_t kBufferSize = 11;
  uint8_t array[kBufferSize];
  PrintToBuffer p2b(array);

  EXPECT_EQ(p2b.print('3'), 1);
  EXPECT_EQ(p2b.print('2'), 1);
  EXPECT_EQ(p2b.print('1'), 1);
  EXPECT_EQ(p2b.print(' '), 1);
  EXPECT_EQ(p2b.print('A'), 1);
  EXPECT_EQ(p2b.print('B'), 1);
  EXPECT_EQ(p2b.print('C'), 1);
  EXPECT_EQ(p2b.print(' '), 1);
  EXPECT_EQ(p2b.print('\0'), 1);
  EXPECT_EQ(p2b.print('\r'), 1);
  EXPECT_EQ(p2b.print("\n"), 1);

  ExpectIsFull(p2b);
  EXPECT_EQ(p2b.ToStringView(), StringView("321 ABC \0\r\n"));
}

TEST(PrintToBufferTest, Prints) {
  constexpr size_t kBufferSize = 16;
  uint8_t array[kBufferSize];
  PrintToBuffer p2b(array);

  EXPECT_EQ(p2b.print(123), 3);
  EXPECT_EQ(p2b.print(' '), 1);
  EXPECT_EQ(p2b.print(MCU_FLASHSTR("abc")), 3);
  EXPECT_EQ(p2b.print(' '), 1);
  EXPECT_EQ(p2b.print("def"), 3);

  EXPECT_EQ(p2b.ToStringView(), StringView("123 abc def"));
}

TEST(PrintToBufferTest, WriteBytesIgnoresEmpty) {
  constexpr size_t kBufferSize = 14;
  uint8_t array[kBufferSize];
  PrintToBuffer p2b(array);

  ExpectIsEmpty(p2b);
  uint8_t array2[kBufferSize];
  EXPECT_EQ(p2b.write(array2, 0), 0);
  ExpectIsEmpty(p2b);
}

TEST(PrintToBufferTest, WriteByteDetectsOverflow) {
  constexpr size_t kBufferSize = 1;
  uint8_t array[kBufferSize];
  PrintToBuffer p2b(array);

  ExpectIsEmpty(p2b);
  EXPECT_EQ(p2b.write(0x41), 1);
  ExpectIsFull(p2b);
  EXPECT_EQ(p2b.ToStringView(), StringView("A"));

  EXPECT_EQ(p2b.write(0x42), 0);

  ExpectHasOverflowed(p2b);
  EXPECT_EQ(p2b.data_size(), 1);
  EXPECT_EQ(p2b.ToStringView(), StringView("A"));
}

TEST(PrintToBufferTest, WriteBytesDetectsOverflow) {
  constexpr size_t kBufferSize = 14;
  uint8_t array[kBufferSize];
  PrintToBuffer p2b(array);
  ExpectIsEmpty(p2b);
  EXPECT_EQ(p2b.print(TEST_STR_16), 0);
  ExpectHasOverflowed(p2b);
  p2b.Reset();
  ExpectIsEmpty(p2b);
  EXPECT_EQ(p2b.print(TEST_STR_14), 14);
  ExpectIsFull(p2b);
  EXPECT_EQ(p2b.ToStringView(), StringView(TEST_STR_14));
}

// Tests of calls to FlushData during overflow.

class MockPrintToBuffer : public PrintToBuffer {
 public:
  using PrintToBuffer::PrintToBuffer;

  // Only mocking the one method we need to intercept.
  MOCK_METHOD(bool, FlushData, (const uint8_t*, size_t), (override));

  void FlushDataToAccumulator(const uint8_t* data, const size_t size) {
    EXPECT_NE(data, nullptr);
    EXPECT_GT(size, 0);
    accumulator_ += std::string_view(reinterpret_cast<const char*>(data), size);
  }

  auto MakeFlushDataAction(bool result = true) {
    return [this, result](const uint8_t* data, size_t size) -> bool {
      this->FlushDataToAccumulator(data, size);
      return result;
    };
  }

  void ExpectFlushData(int times = 1) {
    EXPECT_CALL(*this, FlushData)
        .Times(times)
        .WillRepeatedly(MakeFlushDataAction());
  }

  const std::string& accumulator() const { return accumulator_; }

  std::string_view ToStdStringView() const {
    return MakeStdStringView(ToStringView());
  }

 private:
  std::string accumulator_;
};

TEST(FlushDataTest, WriteByteFlushesWhenFull) {
  constexpr uint8_t kBufferSize = 5;
  uint8_t array[kBufferSize];
  MockPrintToBuffer mp2b(array);

  EXPECT_CALL(mp2b, FlushData).Times(0);
  std::string expected;

  for (uint8_t i = 0; i < kBufferSize; ++i) {
    EXPECT_EQ(mp2b.data_size(), i);
    EXPECT_EQ(mp2b.data_size(), i);
    EXPECT_EQ(MakeStdStringView(mp2b.ToStringView()), expected);
    EXPECT_EQ(mp2b.write(i), 1);
    expected.push_back(i);
  }

  // Next byte should cause an overflow; flush it to the accumulator.
  mp2b.ExpectFlushData();
  EXPECT_EQ(mp2b.write(kBufferSize), 1);
  expected.push_back(kBufferSize);

  EXPECT_EQ(mp2b.data_size(), 1);
  EXPECT_EQ(mp2b.data_size(), 1);
  EXPECT_FALSE(mp2b.HasWriteError());
  EXPECT_EQ(mp2b.availableForWrite(), kBufferSize - 1);

  mp2b.ExpectFlushData();
  mp2b.flush();
  ExpectIsEmpty(mp2b);

  EXPECT_EQ(mp2b.accumulator(), expected);
}

TEST(FlushDataTest, WriteBytesFlushesData) {
  constexpr char kTestString[] = TEST_STR_32;
  constexpr size_t kTestStringSize = sizeof(kTestString) - 1;  // Without NUL.
  constexpr size_t kBufferSize = kTestStringSize - 1;  // Just one too small.
  uint8_t array[kBufferSize];
  MockPrintToBuffer mp2b(array);

  EXPECT_CALL(mp2b, FlushData)
      .Times(AtLeast(1))
      .WillRepeatedly(mp2b.MakeFlushDataAction());

  ExpectIsEmpty(mp2b);
  EXPECT_EQ(mp2b.print(kTestString), kTestStringSize);
  mp2b.flush();
  ExpectIsEmpty(mp2b);

  EXPECT_EQ(mp2b.accumulator(), kTestString);
}

TEST(FlushDataTest, WriteBytesManySizes) {
  const std::string kTestString(TEST_STR_19);
  // Choose a buffer size that is less than 1/3rd the size of the test string
  // so that many boundary cases will be tested.
  constexpr size_t kBufferSize = 5;

  for (size_t input_size = 1; input_size <= kTestString.size(); ++input_size) {
    uint8_t array[kBufferSize];
    MockPrintToBuffer mp2b(array);
    ON_CALL(mp2b, FlushData).WillByDefault(mp2b.MakeFlushDataAction());
    std::string_view remainder = kTestString;
    while (!remainder.empty()) {
      auto sv = MakeStringView(
          remainder.substr(0, std::min(remainder.size(), input_size)));
      EXPECT_EQ(mp2b.write(sv.bytes(), sv.size()), sv.size());
      remainder.remove_prefix(sv.size());
    }
    mp2b.flush();

    EXPECT_EQ(mp2b.accumulator(), kTestString);
  }
}

TEST(FlushDataTest, WriteOneByteFlushDataFails) {
  constexpr uint8_t kBufferSize = 2;
  uint8_t array[kBufferSize];
  MockPrintToBuffer mp2b(array);

  EXPECT_CALL(mp2b, FlushData).Times(0);
  std::string expected;

  for (uint8_t i = 0; i < kBufferSize; ++i) {
    EXPECT_EQ(mp2b.data_size(), i);
    EXPECT_EQ(mp2b.data_size(), i);
    EXPECT_EQ(mp2b.ToStdStringView(), expected);
    EXPECT_EQ(mp2b.write(i), 1);
    expected.push_back(i);
  }

  ExpectIsFull(mp2b);
  EXPECT_EQ(mp2b.ToStdStringView(), expected);

  // Next byte should overflow; since FlushData will return false here, there
  // should be no further calls to FlushData.
  EXPECT_CALL(mp2b, FlushData)
      .WillOnce(DoAll(InvokeWithoutArgs([&]() { ExpectIsFull(mp2b); }),
                      Invoke(mp2b.MakeFlushDataAction(/*result=*/false))));

  EXPECT_EQ(mp2b.write(kBufferSize), 0);
  ExpectHasOverflowed(mp2b);
  EXPECT_EQ(expected, MakeStdStringView(mp2b.ToStringView()));
}

TEST(FlushDataTest, WriteBytesEmptyBufferFails) {
  // Print a string more than double the size of the buffer, so at least two
  // calls to FlushData should be required. But on the first one, return false,
  // so the printing must stop.
  constexpr uint8_t kBufferSize = 5;
  uint8_t array[kBufferSize];
  MockPrintToBuffer mp2b(array);
  // We accumulate the data before returning true, allowing us to confirm that
  // PrintToBuffer attempts to flush the buffer, even if it learns that
  // the flush wasn't successful.
  EXPECT_CALL(mp2b, FlushData)
      .WillOnce(DoAll(Invoke([&](const uint8_t* data, const size_t size) {
                        EXPECT_EQ(data, array);
                      }),
                      Invoke(mp2b.MakeFlushDataAction(/*result=*/false))));
  EXPECT_EQ(mp2b.print('X'), 1);
  EXPECT_EQ(mp2b.print(TEST_STR_16), 0);
  ExpectHasOverflowed(mp2b);
  EXPECT_THAT(absl::StrCat("X", TEST_STR_16), StartsWith(mp2b.accumulator()));
  EXPECT_THAT(mp2b.accumulator(), testing::SizeIs(kBufferSize));
}

TEST(FlushDataTest, WriteBytesFlushDataFails) {
  // Print a string more than double the size of the buffer, so at least two
  // calls to FlushData should be required. But on the second one, return false,
  // so the printing must stop.
  constexpr uint8_t kBufferSize = 5;
  uint8_t array[kBufferSize];
  MockPrintToBuffer mp2b(array);
  {
    InSequence s;
    mp2b.ExpectFlushData();
    // We accumulate the data before returning true, allowing us to confirm that
    // PrintToBuffer attempts to flush the full string, even if it learns that
    // the flush wasn't successful.
    EXPECT_CALL(mp2b, FlushData)
        .WillOnce(DoAll(Invoke([&](const uint8_t* data, const size_t size) {
                          // White box test: The second flush should be from
                          // within the TEST_STR_16, not the buffer space
                          // provided to PrintToBuffer at construction time.
                          EXPECT_NE(data, array);
                        }),
                        Invoke(mp2b.MakeFlushDataAction(/*result=*/false))));
  }
  EXPECT_EQ(mp2b.print('X'), 1);
  EXPECT_EQ(mp2b.print(TEST_STR_16), 0);
  ExpectHasOverflowed(mp2b);
  EXPECT_EQ(mp2b.accumulator(), absl::StrCat("X", TEST_STR_16));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
