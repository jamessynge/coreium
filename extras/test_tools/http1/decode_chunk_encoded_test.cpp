#include "extras/test_tools/http1/decode_chunk_encoded.h"

#include <initializer_list>
#include <string_view>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace mcucore {
namespace test {
namespace {

using ::absl::StatusCode;
using ::testing::HasSubstr;
using ::testing::Pair;
using ::testing::status::IsOkAndHolds;
using ::testing::status::StatusIs;

TEST(DecodeChunkSizeTest, GoodSizes) {
  std::initializer_list<std::pair<std::string_view, size_t>> size_table = {
      {"0", 0},   {"1", 1},   {"2", 2},   {"3", 3},   {"4", 4},   {"5", 5},
      {"6", 6},   {"7", 7},   {"8", 8},   {"9", 9},   {"a", 10},  {"b", 11},
      {"c", 12},  {"d", 13},  {"e", 14},  {"f", 15},  {"A", 10},  {"B", 11},
      {"C", 12},  {"D", 13},  {"E", 14},  {"F", 15},  {"10", 16}, {"11", 17},
      {"12", 18}, {"13", 19}, {"14", 20}, {"15", 21}, {"16", 22}, {"17", 23},
      {"18", 24}, {"19", 25}, {"1a", 26}, {"1b", 27}, {"1c", 28}, {"1d", 29},
      {"1e", 30}, {"1f", 31}, {"1A", 26}, {"1B", 27}, {"1C", 28}, {"1D", 29},
      {"1E", 30}, {"1F", 31}, {"20", 32}, {"21", 33}, {"22", 34}, {"23", 35},
  };
  for (const auto [size_str, size] : size_table) {
    for (const auto after_size :
         {"", "\r\n", "0\r\n", "\r\n\r\n", "1", "F", "Z"}) {
      const auto encoded = absl::StrCat(size_str, "\r\n", after_size);
      EXPECT_THAT(DecodeChunkSize(encoded),
                  IsOkAndHolds(Pair(size, after_size)))
          << "\rencoded: " << encoded;
    }
  }
  EXPECT_THAT(DecodeChunkSize("4\r\nWiki\r\n6"),
              IsOkAndHolds(Pair(4, "Wiki\r\n6")));
  EXPECT_THAT(DecodeChunkSize("6\r\npedi"), IsOkAndHolds(Pair(6, "pedi")));
  EXPECT_THAT(DecodeChunkSize("E\r\nin \r"), IsOkAndHolds(Pair(14, "in \r")));
  EXPECT_THAT(DecodeChunkSize("0\r\n\r\n"), IsOkAndHolds(Pair(0, "\r\n")));
}

TEST(DecodeChunkSizeTest, BadSizes) {
  EXPECT_THAT(DecodeChunkSize(""),
              StatusIs(StatusCode::kInvalidArgument,
                       "Expected chunk, got empty string."));
  EXPECT_THAT(DecodeChunkSize("aaaaaa"),
              StatusIs(StatusCode::kInvalidArgument,
                       HasSubstr("Expected chunk size to end: aaaaaa")));
  EXPECT_THAT(DecodeChunkSize("XXX"),
              StatusIs(StatusCode::kInvalidArgument,
                       HasSubstr("Expected chunk size: XXX")));
  EXPECT_THAT(DecodeChunkSize("0x"),
              StatusIs(StatusCode::kInvalidArgument,
                       HasSubstr("String too short (1) for CRLF: x")));
  EXPECT_THAT(DecodeChunkSize("0\r"),
              StatusIs(StatusCode::kInvalidArgument,
                       HasSubstr("String too short (1) for CRLF: \r")));
  EXPECT_THAT(DecodeChunkSize("0xx"), StatusIs(StatusCode::kInvalidArgument,
                                               HasSubstr("Expected CRLF: xx")));
  EXPECT_THAT(
      DecodeChunkSize("111\rn"),
      StatusIs(StatusCode::kInvalidArgument, HasSubstr("Expected CRLF: \rn")));
  EXPECT_THAT(DecodeChunkSize("aaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n"),
              StatusIs(StatusCode::kInvalidArgument,
                       HasSubstr("Failed to convert with SimpleHexAtoi: "
                                 "aaaaaaaaaaaaaaaaaaaaaaaaaaa")));
}

TEST(DecodeOneChunkTest, GoodChunks) {
  std::initializer_list<std::pair<std::string_view, std::string_view>>
      size_str_and_data = {{"4", "Wiki"},
                           {"6", "pedia "},
                           {"E", "in \r\n\r\nchunks."},
                           {"0", ""}};
  for (const auto [size_str, data] : size_str_and_data) {
    for (const auto after_size :
         {"", "\r\n", "0\r\n", "\r\n\r\n", "1", "F", "Z"}) {
      const auto encoded =
          absl::StrCat(size_str, "\r\n", data, "\r\n", after_size);
      EXPECT_THAT(DecodeOneChunk(encoded), IsOkAndHolds(Pair(data, after_size)))
          << "\rencoded: " << encoded;
    }
  }
}

TEST(DecodeOneChunkTest, BadChunks) {
  EXPECT_THAT(DecodeOneChunk(""),
              StatusIs(StatusCode::kInvalidArgument,
                       "Expected chunk, got empty string."));
  EXPECT_THAT(DecodeOneChunk("F\r\n"),
              StatusIs(StatusCode::kInvalidArgument,
                       "Chunk size larger than available (15 > 0): "));
  EXPECT_THAT(DecodeOneChunk("2\r\n1"),
              StatusIs(StatusCode::kInvalidArgument,
                       "Chunk size larger than available (2 > 1): 1"));
  EXPECT_THAT(DecodeOneChunk("2\r\n12"),
              StatusIs(StatusCode::kInvalidArgument,
                       "String too short (0) for CRLF: "));
  EXPECT_THAT(DecodeOneChunk("2\r\n123"),
              StatusIs(StatusCode::kInvalidArgument,
                       "String too short (1) for CRLF: 3"));
  EXPECT_THAT(DecodeOneChunk("2\r\n1234"),
              StatusIs(StatusCode::kInvalidArgument, "Expected CRLF: 34"));
  EXPECT_THAT(DecodeOneChunk("2\r\n12\rn"),
              StatusIs(StatusCode::kInvalidArgument, "Expected CRLF: \rn"));
}

TEST(DecodeChunkEncodedTest, WikipediaInChunks) {
  // Example from here: https://en.wikipedia.org/wiki/Chunked_transfer_encoding
  const char kTestString[] =
      "4\r\nWiki\r\n6\r\npedia \r\nE\r\nin \r\n\r\nchunks.\r\n0\r\n\r\n";
  EXPECT_THAT(DecodeChunkEncoded(kTestString),
              IsOkAndHolds(Pair("Wikipedia in \r\n\r\nchunks.", "")));

  const char kExtraString[] = "a\r\nOtherStuff\r\n";
  EXPECT_THAT(DecodeChunkEncoded(absl::StrCat(kTestString, kExtraString)),
              IsOkAndHolds(Pair("Wikipedia in \r\n\r\nchunks.", kExtraString)));
}

TEST(DecodeChunkEncodedTest, Empty) {
  const char kTestString[] = "0\r\n\r\n";
  EXPECT_THAT(DecodeChunkEncoded(kTestString), IsOkAndHolds(Pair("", "")));

  const char kExtraString[] = "a\r\nOtherStuff\r\n";
  EXPECT_THAT(DecodeChunkEncoded(absl::StrCat(kTestString, kExtraString)),
              IsOkAndHolds(Pair("", kExtraString)));
}

TEST(DecodeChunkEncodedTest, MissingLastChunk) {
  const char kTestString[] =
      "4\r\nWiki\r\n6\r\npedia \r\nE\r\nin \r\n\r\nchunks.\r\n";
  EXPECT_THAT(
      DecodeChunkEncoded(kTestString),
      StatusIs(
          StatusCode::kInvalidArgument,
          "Found no last chunk, after decoding: Wikipedia in \r\n\r\nchunks."));
}

TEST(DecodeChunkEncodedTest, ShortChunk) {
  const char kTestString[] = "4\r\nWiki\r\n6\r\npedia \r\nE\r\nin \r\n\r";
  EXPECT_THAT(DecodeChunkEncoded(kTestString),
              StatusIs(StatusCode::kInvalidArgument,
                       HasSubstr("Chunk size larger than available (14 >")));
}

}  // namespace
}  // namespace test
}  // namespace mcucore
