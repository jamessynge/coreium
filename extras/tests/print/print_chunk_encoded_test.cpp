#include "print/print_chunk_encoded.h"

#include <string_view>

#include "extras/test_tools/http1/decode_chunk_encoded.h"
#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/test_strings.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

using ::mcucore::test::PrintToStdString;
using ::testing::EndsWith;
using ::testing::Pair;
using ::testing::status::IsOkAndHolds;

TEST(PrintChunkEncodedTest, Empty) {
  PrintToStdString p2ss;
  {
    constexpr size_t kBufferSize = 128;  // Any size OK for this test.
    uint8_t array[kBufferSize];
    PrintChunkEncoded pce(array, p2ss);
  }
  EXPECT_EQ(p2ss.str(), "0\r\n\r\n");
}

TEST(PrintChunkEncodedTest, WikipediaInChunksExtraFlushOk) {
  // Example from here: https://en.wikipedia.org/wiki/Chunked_transfer_encoding
  PrintToStdString p2ss;
  {
    constexpr size_t kBufferSize = 14;  // Largest size used.
    uint8_t array[kBufferSize];
    PrintChunkEncoded pce(array, p2ss);
    pce.print("Wiki");
    pce.flush();
    pce.print("pedia ");
    pce.flush();
    pce.flush();  // Extra flush ignored.
    pce.print("in \r\n\r\nchunks.");
  }
  EXPECT_EQ(p2ss.str(),
            "4\r\nWiki\r\n6\r\npedia \r\nE\r\nin \r\n\r\nchunks.\r\n0\r\n\r\n");
}

TEST(PrintChunkEncodedTest, WikipediaInChunksFinalFlushOk) {
  // Example from here: https://en.wikipedia.org/wiki/Chunked_transfer_encoding
  PrintToStdString p2ss;
  {
    constexpr size_t kBufferSize = 14;  // Largest size used.
    uint8_t array[kBufferSize];
    PrintChunkEncoded pce(array, p2ss);
    pce.print("Wiki");
    pce.flush();
    pce.print("pedia ");
    pce.flush();
    pce.print("in \r\n\r\nchunks.");
    pce.flush();  // Not required, but OK.
  }
  EXPECT_EQ(p2ss.str(),
            "4\r\nWiki\r\n6\r\npedia \r\nE\r\nin \r\n\r\nchunks.\r\n0\r\n\r\n");
}

TEST(PrintChunkEncodedTest, VariousLengths) {
  const char kTestString[] = TEST_STR_1023;
  PrintToStdString p2ss;
  {
    constexpr size_t kBufferSize = 64;
    uint8_t array[kBufferSize];
    PrintChunkEncoded pce(array, p2ss);
    std::string_view str(kTestString);
    size_t size = 0;
    while (!str.empty()) {
      ++size;
      if (size > str.size()) {
        size = str.size();
      }
      EXPECT_EQ(pce.write(reinterpret_cast<const uint8_t*>(str.data()), size),
                size);
      str.remove_prefix(size);
    }
  }
  EXPECT_THAT(p2ss.str(), EndsWith("\r\n0\r\n\r\n"));
  EXPECT_THAT(DecodeChunkEncoded(p2ss.str()),
              IsOkAndHolds(Pair(kTestString, "")));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
