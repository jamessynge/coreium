#include "any_printable.h"

#include <stddef.h>
#include <stdint.h>

#include <cstring>
#include <string>

#include "extras/host/arduino/print.h"
#include "extras/test_tools/print_to_std_string.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "progmem_string_data.h"
#include "progmem_string_view.h"
#include "string_view.h"

namespace mcucore {
namespace test {
namespace {

#ifdef TEST_TEMPORARY_PRINTABLE
// For testing that we can't pass temporary Printable instances into an
// AnyPrintable constructor. See these pages for ideas on how to add a negative
// compilation tests to code that actually does compile:
//
// https://akrzemi1.wordpress.com/2016/05/10/diagnosable-validity/
//
// https://coderedirect.com/questions/553155/sfinae-to-assert-that-code-does-not-compile
//
// https://stackoverflow.com/questions/23547831/assert-that-code-does-not-compile
TEST(AnyPrintableTest, PrintableNegativeCompilation) {
  // None of these lines should compile.
  auto a = AnyPrintable{SamplePrintable()};
  auto b = AnyPrintable(SamplePrintable());
  AnyPrintable c = SamplePrintable();
  AnyPrintable d((SamplePrintable()));
}
#endif

std::string AnyPrintableToString(const AnyPrintable& any_printable) {
  mcucore::test::PrintToStdString out;
  const size_t count = out.print(any_printable);
  const std::string result = out.str();
  EXPECT_EQ(count, result.size());
  // Test copying.
  {
    AnyPrintable copy = any_printable;
    mcucore::test::PrintToStdString out2;
    EXPECT_EQ(out2.print(copy), count);
    EXPECT_EQ(out2.str(), result);
  }
  return result;
}

template <typename T>
std::string PrintConstRefViaAnyPrintable(const T& t) {
  AnyPrintable any_printable(t);
  return AnyPrintableToString(any_printable);
}

template <typename T>
std::string PrintRefViaAnyPrintable(T& t) {
  AnyPrintable any_printable(t);
  const auto result = AnyPrintableToString(any_printable);
  // We have a ref to the value, so we can test AnyPrintable's ctor can accept
  // a const ref to that value.
  EXPECT_EQ(PrintConstRefViaAnyPrintable(t), result);
  return result;
}

template <typename T>
std::string PrintValueViaAnyPrintable(T t) {
  AnyPrintable any_printable(t);
  const auto result = AnyPrintableToString(any_printable);
  // We have a copy of the value, so we can test AnyPrintable's ctor can accept
  // a ref to that value.
  EXPECT_EQ(PrintRefViaAnyPrintable(t), result);
  return result;
}

TEST(AnyPrintableTest, Empty) {
  AnyPrintable any_printable;
  EXPECT_THAT(AnyPrintableToString(any_printable), testing::IsEmpty());
}

// This provides coverage of various Literal and StringView methods.
template <int N>
void VerifyStringLiteralPrinting(const char (&buf)[N]) {
  const std::string expected(buf);
  EXPECT_EQ(expected.size(), N - 1);

  {
    ProgmemStringView value(buf);
    EXPECT_EQ(value.size(), expected.size());
    EXPECT_EQ(AnyPrintableToString(value), expected);
  }

  {
    StringView value(buf);
    EXPECT_EQ(value.size(), expected.size());
    EXPECT_EQ(AnyPrintableToString(value), expected);
  }
}

TEST(AnyPrintableTest, StringLiterals) {
  VerifyStringLiteralPrinting("");
  VerifyStringLiteralPrinting(" ");
  VerifyStringLiteralPrinting("some literal text");
}

void VerifyFlashStringPrinting(const __FlashStringHelper* str,
                               const std::string expected) {
  EXPECT_EQ(std::strlen(reinterpret_cast<const char*>(str)), expected.size());
  EXPECT_EQ(PrintValueViaAnyPrintable(str), expected);
}

#define VERIFY_FLASH_STRING_PRINTING(string_literal)      \
  VerifyFlashStringPrinting(MCU_FLASHSTR(string_literal), \
                            std::string(string_literal))

TEST(AnyPrintableTest, FlashStrings) {
  VERIFY_FLASH_STRING_PRINTING("");
  VERIFY_FLASH_STRING_PRINTING(" ");
  VERIFY_FLASH_STRING_PRINTING("some literal text");
}

TEST(AnyPrintableTest, PrintableReference) {
  constexpr char kStr[] = "some more text";
  StringView view(kStr);
  AnyPrintable printable1(view);
  Printable& printable2 = printable1;
  AnyPrintable printable3(printable2);
  EXPECT_EQ(AnyPrintableToString(printable3), kStr);
}

TEST(AnyPrintableTest, ConstPrintableReference) {
  constexpr char kStr[] = "some other text";
  const StringView view(kStr);
  const AnyPrintable printable1(view);
  const Printable& printable2 = printable1;
  const AnyPrintable printable3(printable2);
  EXPECT_EQ(AnyPrintableToString(printable3), kStr);
}

TEST(AnyPrintableTest, ManyTypes) {
  {
    char v = '*';
    EXPECT_EQ(PrintRefViaAnyPrintable(v), "*");
    EXPECT_EQ(PrintValueViaAnyPrintable(v), "*");
    EXPECT_EQ(PrintValueViaAnyPrintable<char>(' '), " ");
    EXPECT_EQ(PrintValueViaAnyPrintable('&'), "&");
  }

  {
    int16_t v = -32768;
    EXPECT_EQ(PrintRefViaAnyPrintable<int16_t>(v), "-32768");
    EXPECT_EQ(PrintValueViaAnyPrintable<int16_t>(v), "-32768");
    EXPECT_EQ(PrintValueViaAnyPrintable<int16_t>(-32768), "-32768");
  }

  {
    int16_t v = 32767;
    EXPECT_EQ(PrintRefViaAnyPrintable<int16_t>(v), "32767");
    EXPECT_EQ(PrintValueViaAnyPrintable<int16_t>(v), "32767");
    EXPECT_EQ(PrintValueViaAnyPrintable<int16_t>(32767), "32767");
  }

  {
    uint16_t v = 65535;
    EXPECT_EQ(PrintRefViaAnyPrintable<uint16_t>(v), "65535");
    EXPECT_EQ(PrintValueViaAnyPrintable<uint16_t>(v), "65535");
    EXPECT_EQ(PrintValueViaAnyPrintable<uint16_t>(65535), "65535");
  }

  {
    int32_t v = -345;
    EXPECT_EQ(PrintRefViaAnyPrintable(v), "-345");
    EXPECT_EQ(PrintValueViaAnyPrintable(v), "-345");
    EXPECT_EQ(PrintValueViaAnyPrintable(-345), "-345");

    EXPECT_EQ(PrintValueViaAnyPrintable<int32_t>(123), "123");
    EXPECT_EQ(PrintValueViaAnyPrintable(123), "123");
  }

  {
    uint32_t v = 12345678;
    EXPECT_EQ(PrintRefViaAnyPrintable(v), "12345678");
    EXPECT_EQ(PrintValueViaAnyPrintable(v), "12345678");
    EXPECT_EQ(PrintValueViaAnyPrintable(1234U), "1234");
  }

  {
    float v = 3.1415f;
    EXPECT_EQ(PrintRefViaAnyPrintable(v), "3.14");
    EXPECT_EQ(PrintValueViaAnyPrintable(v), "3.14");
    EXPECT_EQ(PrintValueViaAnyPrintable(3.14f), "3.14");
  }

  {
    double v = 2.71828;
    EXPECT_EQ(PrintRefViaAnyPrintable(v), "2.72");
    EXPECT_EQ(PrintValueViaAnyPrintable(v), "2.72");
    EXPECT_EQ(PrintValueViaAnyPrintable(2.71828), "2.72");
  }

  {
    StringView v("some_text");
    EXPECT_EQ(PrintRefViaAnyPrintable(v), "some_text");
    EXPECT_EQ(PrintValueViaAnyPrintable(v), "some_text");
    EXPECT_EQ(PrintValueViaAnyPrintable(AnyPrintable(v)), "some_text");
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
