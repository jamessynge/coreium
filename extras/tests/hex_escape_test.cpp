#include "hex_escape.h"

#include <stddef.h>

#include <cstdio>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/sample_printable.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "string_view.h"
#include "util/gtl/map_util.h"

namespace mcucore {
namespace test {
namespace {

using ::testing::Contains;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::SizeIs;

// Support for logging by Google Test (e.g. EXPECT_EQ).
std::ostream& operator<<(std::ostream& os, EHexEscapingState v) {
  switch (v) {
    case EHexEscapingState::kNormal:
      return os << "kNormal";
    case EHexEscapingState::kHexDigitOutput:
      return os << "kHexDigitOutput";
    case EHexEscapingState::kQuestionMarkOutput:
      return os << "kQuestionMarkOutput";
  }
  return os << "Unknown {name}, value=" << static_cast<int64_t>(v);
}

std::string HexEncodeOneChar(const char c) {
  char buffer[16];
  std::string_view expected(buffer,
                            std::snprintf(buffer, sizeof buffer, "\\x%02X",
                                          static_cast<unsigned int>(c + 0)));
  return std::string(expected);
}

std::pair<std::string, EHexEscapingState> PrintOneCharWithEscapingState(
    const char c, EHexEscapingState state) {
  PrintToStdString p2ss;
  const auto size = PrintCharWithStateHexEscaped(p2ss, c, state);
  const auto printed = p2ss.str();
  EXPECT_EQ(size, printed.size());
  return std::make_pair(printed, state);
}

TEST(PrintCharHexEscapedTest, TestAll) {
  // Initialize with those characters that always have a special representation.
  const std::vector<std::pair<char, std::string>> test_cases = {
      // clang-format off
      {'\n', "\\n"},
      {'\012', "\\n"},
      {'\x0a', "\\n"},

      {'\r', "\\r"},
      {'\015', "\\r"},
      {'\x0d', "\\r"},

      {'\\', "\\\\"},
      {'"', "\\\""},
      // clang-format on
  };
  std::set<char> tested;
  for (auto [c, expected] : test_cases) {
    tested.insert(c);
    PrintToStdString out;
    PrintCharHexEscaped(out, c);
    EXPECT_EQ(out.str(), expected);
  }
  for (char c = ' '; c < 127; ++c) {
    if (c == '"' || c == '\\') {
      EXPECT_THAT(tested, Contains(c));
    } else {
      EXPECT_THAT(tested, Not(Contains(c)));
      tested.insert(c);
      PrintToStdString out;
      PrintCharHexEscaped(out, c);
      EXPECT_THAT(out.str(), SizeIs(1));
      EXPECT_EQ(out.str().at(0), c);
    }
  }
  for (int i = -128; i <= 127; ++i) {
    char c = static_cast<char>(i);
    if (!gtl::ContainsKey(tested, c)) {
      tested.insert(c);
      PrintToStdString out;
      PrintCharHexEscaped(out, c);
      EXPECT_THAT(out.str(), SizeIs(4));
      char buffer[16];
      std::string_view expected(
          buffer, std::snprintf(buffer, sizeof buffer, "\\x%02X",
                                static_cast<unsigned int>(c + 0)));
      EXPECT_THAT(out.str(), expected);
    }
  }
  EXPECT_THAT(tested, SizeIs(256));
}

TEST(PrintCharWithStateHexEscapedTest, HexDigitEscaped) {
  // Ensure that if the preceding character was output hex escaped, then the
  // following character will be hex escaped if it is a hex digit.

  for (const char c : std::string("0123456789ABCDEFabcdef")) {
    EXPECT_EQ(PrintOneCharWithEscapingState(c, EHexEscapingState::kNormal),
              std::make_pair(std::string({c}), EHexEscapingState::kNormal));

    EXPECT_EQ(PrintOneCharWithEscapingState(
                  c, EHexEscapingState::kQuestionMarkOutput),
              std::make_pair(std::string({c}), EHexEscapingState::kNormal));

    EXPECT_EQ(
        PrintOneCharWithEscapingState(c, EHexEscapingState::kHexDigitOutput),
        std::make_pair(HexEncodeOneChar(c),
                       EHexEscapingState::kHexDigitOutput));
  }

  // If we do the same with non-hex digits, the characters shouldn't be escaped.

  for (const char c : std::string("!\"#$%&'()*+,-./:;<=>?@[]\\^_`{}|~")) {
    const std::string expected_str =
        (c == '"' || c == '\\') ? std::string({'\\', c}) : std::string({c});
    const auto expected_state = c == '?'
                                    ? EHexEscapingState::kQuestionMarkOutput
                                    : EHexEscapingState::kNormal;

    EXPECT_EQ(PrintOneCharWithEscapingState(c, EHexEscapingState::kNormal),
              std::make_pair(expected_str, expected_state));
    EXPECT_EQ(
        PrintOneCharWithEscapingState(c, EHexEscapingState::kHexDigitOutput),
        std::make_pair(expected_str, expected_state));
  }
}

TEST(PrintCharWithStateHexEscapedTest, TrigraphEscaped) {
  // Ensure that if multiple question marks are emitted in a row, all but the
  // first one is escaped. This supports trigraphs in C++ prior to version 2017;
  // well, actually, it supports NOT emitting trigraphs.

  EXPECT_EQ(
      PrintOneCharWithEscapingState('?', EHexEscapingState::kNormal),
      std::make_pair(std::string("?"), EHexEscapingState::kQuestionMarkOutput));

  EXPECT_EQ(
      PrintOneCharWithEscapingState('?', EHexEscapingState::kHexDigitOutput),
      std::make_pair(std::string("?"), EHexEscapingState::kQuestionMarkOutput));

  EXPECT_EQ(PrintOneCharWithEscapingState(
                '?', EHexEscapingState::kQuestionMarkOutput),
            std::make_pair(std::string("\\?"),
                           EHexEscapingState::kQuestionMarkOutput));

  // If a character other than a question mark is printed after a question mark,
  // the output state should NOT be kQuestionMarkOutput.

  for (char c = ' '; c <= '~'; ++c) {
    if (c == '?') {
      continue;
    }

    auto result = PrintOneCharWithEscapingState(
        c, EHexEscapingState::kQuestionMarkOutput);
    EXPECT_EQ(result.second, EHexEscapingState::kNormal);
  }
}

TEST(PrintCharWithStateHexEscapedTest, TestAllSingular) {
  // Initialize with those characters that always have a special representation.
  const std::vector<std::pair<char, std::string>> test_cases = {
      // clang-format off
      {'\n', "\\n"},
      {'\012', "\\n"},
      {'\x0a', "\\n"},

      {'\r', "\\r"},
      {'\015', "\\r"},
      {'\x0d', "\\r"},

      {'\\', "\\\\"},
      {'"', "\\\""},
      // clang-format on
  };
  std::set<char> tested;
  for (auto [c, expected] : test_cases) {
    tested.insert(c);
    PrintToStdString out;
    PrintCharHexEscaped(out, c);
    EXPECT_EQ(out.str(), expected);
  }
  for (char c = ' '; c < 127; ++c) {
    if (c == '"' || c == '\\') {
      EXPECT_THAT(tested, Contains(c));
    } else {
      EXPECT_THAT(tested, Not(Contains(c)));
      tested.insert(c);
      PrintToStdString out;
      PrintCharHexEscaped(out, c);
      EXPECT_THAT(out.str(), SizeIs(1));
      EXPECT_EQ(out.str().at(0), c);
    }
  }
  for (int i = -128; i <= 127; ++i) {
    char c = static_cast<char>(i);
    if (!gtl::ContainsKey(tested, c)) {
      tested.insert(c);
      PrintToStdString out;
      PrintCharHexEscaped(out, c);
      EXPECT_THAT(out.str(), SizeIs(4));
      char buffer[16];
      std::string_view expected(
          buffer, std::snprintf(buffer, sizeof buffer, "\\x%02X",
                                static_cast<unsigned int>(c + 0)));
      EXPECT_THAT(out.str(), expected);
    }
  }
  EXPECT_THAT(tested, SizeIs(256));
}

std::string PrintHexEscapedTwoWays(const std::string& str) {
  std::string whole_string, char_at_a_time;
  {
    PrintToStdString inner;
    PrintHexEscaped outer(inner);
    size_t size = outer.print(str.data());
    whole_string = inner.str();
    EXPECT_EQ(whole_string.size(), size);
  }
  {
    PrintToStdString inner;
    PrintHexEscaped outer(inner);
    size_t size = 0;
    for (const char c : str) {
      size += outer.print(c);
    }
    char_at_a_time = inner.str();
    EXPECT_EQ(char_at_a_time.size(), size);
  }
  EXPECT_EQ(whole_string, char_at_a_time) << "\nFor string: " << str;
  return whole_string;
}

TEST(PrintHexEscapedTest, EmptyString) {
  EXPECT_THAT(PrintHexEscapedTwoWays(""), IsEmpty());
}

constexpr char kStringLiteral[] =
    "<tag attr=\"value with slash ('\\')\">\b\f\n\r\t</tag>";
constexpr char kEscapedLiteral[] =
    "<tag attr=\\\"value with slash ('\\\\')\\\">"
    "\\x08\\x0C\\n\\r\\x09</tag>";
constexpr char kQuotedEscapedLiteral[] =
    "\"<tag attr=\\\"value with slash ('\\\\')\\\">"
    "\\x08\\x0C\\n\\r\\x09</tag>\"";

TEST(PrintHexEscapedTest, CLiteralStringWithEscapes) {
  EXPECT_THAT(PrintHexEscapedTwoWays(kStringLiteral), kEscapedLiteral);
}

TEST(HexEscapedPrintableTest, PrintableWithEscapes) {
  SamplePrintable original("abc\r\n");
  auto printable = HexEscapedPrintable<SamplePrintable>(original);

  PrintToStdString out;
  const size_t count = printable.printTo(out);

  const std::string expected = "\"abc\\r\\n\"";
  EXPECT_EQ(out.str(), expected);
  EXPECT_EQ(count, expected.size());
}

TEST(HexEscapedPrintableTest, StringViewWithEscapes) {
  StringView original(kStringLiteral);
  auto printable = HexEscapedPrintable<StringView>(original);

  PrintToStdString out;
  const size_t count = printable.printTo(out);

  const std::string expected = kQuotedEscapedLiteral;
  EXPECT_EQ(out.str(), expected);
  EXPECT_EQ(count, expected.size());
}

TEST(HexEscapedPrintableTest, LiteralWithEscapes) {
  ProgmemStringView original(kStringLiteral);
  auto printable = HexEscapedPrintable<ProgmemStringView>(original);

  PrintToStdString out;
  const size_t count = printable.printTo(out);

  const std::string expected = kQuotedEscapedLiteral;
  EXPECT_EQ(out.str(), expected);
  EXPECT_EQ(count, expected.size());
}

TEST(HexEscapedTest, PrintableWithEscapes) {
  SamplePrintable original("abc\r\n");
  auto printable = HexEscaped(original);

  PrintToStdString out;
  const size_t count = out.print(printable);

  const std::string expected = "\"abc\\r\\n\"";
  EXPECT_EQ(out.str(), expected);
  EXPECT_EQ(count, expected.size());
}

TEST(HexEscapedTest, LiteralWithEscapes) {
  ProgmemStringView original(kStringLiteral);
  auto printable = HexEscaped(original);

  PrintToStdString out;
  const size_t count = out.print(printable);

  const std::string expected = kQuotedEscapedLiteral;
  EXPECT_EQ(out.str(), expected);
  EXPECT_EQ(count, expected.size());
}

}  // namespace
}  // namespace test
}  // namespace mcucore
