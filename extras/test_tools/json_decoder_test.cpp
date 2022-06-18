#include "extras/test_tools/json_decoder.h"

#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/task/status_macros.h"

namespace mcucore {
namespace test {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::Pair;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;
using ::testing::status::IsOk;
using ::testing::status::IsOkAndHolds;
using ::testing::status::StatusIs;

#define WHITESPACE " \n\r\t"

void VerifyHandlesBogusAccessors(const JsonValue& value) {
  EXPECT_FALSE(value.HasIndex(999999));
  EXPECT_FALSE(value.HasKey(" a * never * used key"));
  EXPECT_THAT(value.HasKeyOfType(" a * never * used key", JsonValue::kUnset),
              Not(IsOk()));

  EXPECT_EQ(value.GetElement(999999).type(), JsonValue::kUnset);
  EXPECT_EQ(value.GetValue(" a * never * used key").type(), JsonValue::kUnset);
  EXPECT_THAT(value.GetValueOfType(" a * never * used key", JsonValue::kUnset),
              Not(IsOk()));
}

TEST(JsonDecoderTest, Unset) {
  JsonValue value;
  EXPECT_EQ(value.type(), JsonValue::kUnset);
  EXPECT_TRUE(value.is_unset());
  EXPECT_EQ(value, JsonValue());
  EXPECT_NE(value, JsonValue(nullptr));
  VerifyHandlesBogusAccessors(value);
  EXPECT_NE(value, 0);
  EXPECT_NE(0.0, value);
}

TEST(JsonDecoderTest, Null) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(WHITESPACE "null" WHITESPACE));
  EXPECT_EQ(value.type(), JsonValue::kNull);
  VerifyHandlesBogusAccessors(value);

  EXPECT_EQ(value, JsonValue(nullptr));
  EXPECT_EQ(JsonValue(nullptr), value);

  EXPECT_EQ(value, nullptr);
  EXPECT_EQ(nullptr, value);

  EXPECT_NE(value, JsonValue(false));
  EXPECT_NE(JsonValue(""), value);
}

TEST(JsonDecoderTest, True) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(WHITESPACE "true" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kBool);
  VerifyHandlesBogusAccessors(value);

  EXPECT_EQ(value.as_bool(), true);
  EXPECT_EQ(true, value.as_bool());
  EXPECT_EQ(value, JsonValue(true));
  EXPECT_EQ(JsonValue(true), value);

  EXPECT_NE(value, JsonValue(nullptr));
  EXPECT_NE(value, false);
}

TEST(JsonDecoderTest, False) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(WHITESPACE "false" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kBool);
  VerifyHandlesBogusAccessors(value);

  EXPECT_EQ(value.as_bool(), false);
  EXPECT_EQ(false, value.as_bool());
  EXPECT_EQ(value, JsonValue(false));
  EXPECT_EQ(JsonValue(false), value);

  EXPECT_NE(value, JsonValue("false"));
  EXPECT_NE(value, true);
}

TEST(JsonDecoderTest, IntegerZero) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(WHITESPACE "-0" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kInteger);
  VerifyHandlesBogusAccessors(value);

  EXPECT_EQ(value.as_integer(), -0);
  EXPECT_EQ(value, JsonValue(-0));
  EXPECT_EQ(JsonValue(-0), value);
  EXPECT_EQ(value, -0);
  EXPECT_EQ(value, 0);
  EXPECT_EQ(-0, value);
  EXPECT_EQ(0, value);

  // Because twos complement doesn't have an explicit representation for -0, we
  // expect that it is identical to +0.
  EXPECT_EQ(value, JsonValue(0));
  EXPECT_EQ(JsonValue(0), value);

  EXPECT_NE(value, JsonValue(nullptr));
  EXPECT_NE(value, 0.0000000001);
  EXPECT_NE(value, -0.0000000001);
  EXPECT_EQ(value, 0.0);
  EXPECT_EQ(value, -0.0);
}

TEST(JsonDecoderTest, IntegerMinusOne) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(WHITESPACE "-1" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kInteger);
  VerifyHandlesBogusAccessors(value);

  EXPECT_EQ(-1, value.as_integer());
  EXPECT_EQ(value.as_integer(), -1);
  EXPECT_EQ(value, JsonValue(-1));
  EXPECT_EQ(JsonValue(-1), value);
}

TEST(JsonDecoderTest, DoubleZero) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(WHITESPACE "-0.0E+0" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kDouble);
  VerifyHandlesBogusAccessors(value);

  EXPECT_EQ(value, 0.0);
  EXPECT_EQ(value, -0.0);
  EXPECT_EQ(value.as_double(), -0.0);
  EXPECT_EQ(value, JsonValue(-0.0));
  EXPECT_EQ(JsonValue(-0.0), value);
  EXPECT_EQ(value, -0);
  EXPECT_EQ(value, 0);
  EXPECT_EQ(-0, value);
  EXPECT_EQ(0, value);

  // Defined -0 == +0.
  EXPECT_EQ(value, JsonValue(0.0));
  EXPECT_EQ(JsonValue(0.0), value);

  EXPECT_NE(value, JsonValue(nullptr));
  EXPECT_NE(value, 0.0000000001);
  EXPECT_NE(value, -0.0000000001);
}

TEST(JsonDecoderTest, DoubleOne) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(WHITESPACE "1.0e-0" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kDouble);
  VerifyHandlesBogusAccessors(value);

  EXPECT_EQ(1.0, value.as_double());
  EXPECT_EQ(value.as_double(), 1.0);
  EXPECT_EQ(value, JsonValue(1.0));
  EXPECT_EQ(JsonValue(1.0), value);

  // Compare to equivalent integer.
  EXPECT_EQ(1, value.as_double());
  EXPECT_EQ(value.as_double(), 1);
}

TEST(JsonDecoderTest, EmptyString) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(WHITESPACE "\"\"" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kString);
  VerifyHandlesBogusAccessors(value);
  EXPECT_EQ(value.size(), 0);

  EXPECT_EQ(value.as_string(), "");
  EXPECT_EQ("", value.as_string());
  EXPECT_EQ(value, JsonValue(""));
  EXPECT_EQ(JsonValue(std::string_view("")), value);
}

TEST(JsonDecoderTest, AllSupportedAsciiCharsString) {
  ASSERT_OK_AND_ASSIGN(
      auto value,
      JsonValue::Parse(WHITESPACE
                       R"("\"\\\/\b\f\n\r\t !#$%&'()*+,-./0123456789:;<=>?)@)"
                       R"(ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`)"
                       R"(abcdefghijklmnopqrstuvwxyz{|}~")" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kString);
  EXPECT_EQ(value.as_string(),
            "\"\\/\b\f\n\r\t !#$%&'()*+,-./0123456789:;<=>?)@"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`"
            "abcdefghijklmnopqrstuvwxyz{|}~");
  VerifyHandlesBogusAccessors(value);
}

TEST(JsonDecoderTest, EmptyObject) {
  ASSERT_OK_AND_ASSIGN(
      auto value, JsonValue::Parse(WHITESPACE "{" WHITESPACE "}" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kObject);
  VerifyHandlesBogusAccessors(value);
  EXPECT_THAT(value.as_object(), IsEmpty());
  EXPECT_EQ(value.size(), 0);

  ASSERT_OK_AND_ASSIGN(value, JsonValue::Parse("{}"));
  ASSERT_EQ(value.type(), JsonValue::kObject);
  EXPECT_THAT(value.as_object(), IsEmpty());
}

TEST(JsonDecoderTest, ObjectWithOneEntry) {
  ASSERT_OK_AND_ASSIGN(auto value, JsonValue::Parse(R"({ "key" : " " })"));
  ASSERT_EQ(value.type(), JsonValue::kObject);
  VerifyHandlesBogusAccessors(value);
  EXPECT_EQ(value.size(), 1);
  EXPECT_THAT(value.as_object(), SizeIs(1));
  EXPECT_THAT(value.as_object(),
              UnorderedElementsAre(Pair("key", JsonValue(" "))));

  EXPECT_TRUE(value.HasKey("key"));
  EXPECT_EQ(value.GetValue("key"), " ");

  EXPECT_OK(value.HasKeyOfType("key", JsonValue::kString));
  EXPECT_THAT(value.HasKeyOfType("key", JsonValue::kNull),
              StatusIs(absl::StatusCode::kFailedPrecondition,
                       HasSubstr("not of expected type")));
  EXPECT_THAT(value.GetValueOfType("key", JsonValue::kString),
              IsOkAndHolds(Eq(" ")));
  EXPECT_THAT(value.GetValueOfType("key", JsonValue::kUnset),
              StatusIs(absl::StatusCode::kFailedPrecondition,
                       HasSubstr("not of expected type")));
}

TEST(JsonDecoderTest, ObjectWithMultipleEntries) {
  ASSERT_OK_AND_ASSIGN(
      auto value,
      JsonValue::Parse(
          R"({"a":null,"b":true,"c":false,"d":-0.0,"e":"","f":{},"g":[9]})"));
  ASSERT_EQ(value.type(), JsonValue::kObject);
  VerifyHandlesBogusAccessors(value);
  EXPECT_EQ(value.size(), 7);

  const auto expected_object = JsonObject()
                                   .Add("a", nullptr)
                                   .Add("b", true)
                                   .Add("c", false)
                                   .Add("d", -0.0)
                                   .Add("e", "")
                                   .Add("f", JsonObject())
                                   .Add("g", JsonArray().Add(9));
  EXPECT_EQ(value, expected_object);
  EXPECT_EQ(value.as_object(), expected_object);

  const JsonValue expected(expected_object);
  EXPECT_EQ(value, expected);
}

TEST(JsonDecoderTest, EmptyArray) {
  ASSERT_OK_AND_ASSIGN(
      auto value, JsonValue::Parse(WHITESPACE "[" WHITESPACE "]" WHITESPACE));
  ASSERT_EQ(value.type(), JsonValue::kArray);
  VerifyHandlesBogusAccessors(value);
  EXPECT_THAT(value.as_array(), IsEmpty());
  EXPECT_EQ(value.size(), 0);

  ASSERT_OK_AND_ASSIGN(value, JsonValue::Parse("[]"));
  ASSERT_EQ(value.type(), JsonValue::kArray);
  EXPECT_THAT(value.as_array(), IsEmpty());

  EXPECT_EQ(value.GetElement(0), JsonValue());
}

TEST(JsonDecoderTest, ArrayWithOneNumberEntry) {
  ASSERT_OK_AND_ASSIGN(auto value, JsonValue::Parse(R"([0.1])"));
  ASSERT_EQ(value.type(), JsonValue::kArray);
  VerifyHandlesBogusAccessors(value);
  EXPECT_THAT(value.as_array(), SizeIs(1));
  EXPECT_EQ(value.size(), 1);
  EXPECT_THAT(value.as_array(), ElementsAre(JsonValue(0.1)));

  EXPECT_EQ(value.GetElement(0), 0.1);
  EXPECT_EQ(value.GetElement(1), JsonValue());
}

TEST(JsonDecoderTest, ArrayWithMultipleEntries) {
  ASSERT_OK_AND_ASSIGN(auto value,
                       JsonValue::Parse(R"([[""],false,null,"",true,100,{}])"));
  ASSERT_EQ(value.type(), JsonValue::kArray);
  VerifyHandlesBogusAccessors(value);
  EXPECT_EQ(value.size(), 7);

  EXPECT_EQ(value.GetElement(0), JsonValue::Parse(R"( [ "" ] )").value());
  EXPECT_EQ(value.GetElement(1), false);
  EXPECT_EQ(value.GetElement(2), nullptr);
  EXPECT_EQ(value.GetElement(3), std::string_view(""));
  EXPECT_EQ(value.GetElement(4), true);
  EXPECT_EQ(value.GetElement(5), 100);
  EXPECT_EQ(value.GetElement(6), JsonObject());
  EXPECT_EQ(value.GetElement(7), JsonValue());

  const auto expected_array = JsonArray()
                                  .Add(JsonArray().Add(""))
                                  .Add(false)
                                  .Add(nullptr)
                                  .Add("")
                                  .Add(true)
                                  .Add(100)
                                  .Add(JsonObject());
  EXPECT_EQ(value, expected_array);
  EXPECT_EQ(value.as_array(), expected_array);

  const JsonValue expected(expected_array);
  EXPECT_EQ(value, expected);
}

TEST(JsonDecoderTest, InvalidStringCharacter) {
  auto tester = [](const char c) {
    std::string s = "\"";
    s += c;
    s += '"';
    EXPECT_THAT(JsonValue::Parse(s),
                StatusIs(absl::StatusCode::kInvalidArgument,
                         HasSubstr("Not a valid string char: ")));
  };

  for (char c = 0; c < ' '; ++c) {
    tester(c);
  }
  for (int c = 127; c < 256; ++c) {
    tester(static_cast<char>(c));
  }
}

TEST(JsonDecoderTest, UnterminatedEscape) {
  EXPECT_THAT(
      JsonValue::Parse("\"\\"),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("Expected escaped character, not end-of-input")));
}
TEST(JsonDecoderTest, InvalidEscape) {
  EXPECT_THAT(JsonValue::Parse("\"\\X abc"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Not a valid string escape: \\X abc")));
}

TEST(JsonDecoderTest, UnicodeNotSupported) {
  EXPECT_THAT(
      JsonValue::Parse("\"\\u123\""),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("Decoding of unicode code points not supported: ")));
}

TEST(JsonDecoderTest, UnterminatedString) {
  EXPECT_THAT(JsonValue::Parse(" \" "),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected end of string, not end-of-input")));
}

TEST(JsonDecoderTest, MalformedNumber) {
  EXPECT_THAT(JsonValue::Parse(" 1+2 "),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected only trailing whitespace")));
  EXPECT_THAT(JsonValue::Parse(" +1 "),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected a number")));
  EXPECT_THAT(JsonValue::Parse(" !1 "),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected a number")));
}

TEST(JsonDecoderTest, MalformedLiteral) {
  EXPECT_THAT(JsonValue::Parse(" nuLL "),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected null, not 'nuLL '")));
  EXPECT_THAT(JsonValue::Parse(" truE "),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected true, not 'truE '")));
  EXPECT_THAT(JsonValue::Parse(" falSe\t"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected false, not 'falSe\t'")));
}

TEST(JsonDecoderTest, MalformedObject) {
  EXPECT_THAT(JsonValue::Parse(" { "),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected a value, not end-of-input")));
  EXPECT_THAT(JsonValue::Parse(R"( { "keyname" )"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected delimiter ':', not end-of-input")));
  EXPECT_THAT(JsonValue::Parse(R"( { "keyname" : )"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected a value, not end-of-input")));
  EXPECT_THAT(JsonValue::Parse(R"( { "keyname" } )"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Expected delimiter ':', not '}'")));
}

TEST(JsonDecoderTest, TwoValuesInARow) {
  {
    const std::vector<std::string> literals = {
        "\"abc\"", "false", "true", "null", "100", "1.0", "{}", "[]",
    };
    for (const auto& literal1 : literals) {
      for (const auto& literal2 : literals) {
        const auto s = absl::StrCat(literal1, " ", literal2);
        EXPECT_THAT(JsonValue::Parse(s),
                    StatusIs(absl::StatusCode::kInvalidArgument,
                             HasSubstr("Expected only trailing whitespace")));
      }
    }
  }
  {
    const std::vector<std::string> literals = {
        "\"abc\"", "false", "true", "null", "-100", "-1.0", "{}", "[]",
    };
    for (const auto& literal1 : literals) {
      for (const auto& literal2 : literals) {
        const auto s = absl::StrCat(literal1, " ", literal2);
        EXPECT_THAT(JsonValue::Parse(s),
                    StatusIs(absl::StatusCode::kInvalidArgument,
                             HasSubstr("Expected only trailing whitespace")));
      }
    }
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
