#include "print/o_print_stream.h"

#include <stdint.h>

#include <iostream>
#include <limits>
#include <string>
#include <string_view>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/sample_printable.h"
#include "extras/test_tools/test_strings.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"
#include "strings/progmem_string.h"
#include "strings/progmem_string_data.h"
#include "strings/progmem_string_view.h"

// It is deliberate that these tests are defined outside of the mcucore
// namespace, as a test of whether OPrintStream can use ADL to locate the
// PrintValueTo function defined for a type (enums in these examples).

namespace ns1 {
enum EScopeLess { kSixteen = 16 };
size_t PrintValueTo(EScopeLess v, Print& out) {
  size_t count = 0;
  if (v == kSixteen) {
    count += out.print("kSixteen");
  } else {
    count += out.print("Unknown: EScopeLess(");
    count += out.print(static_cast<int>(v), 10);
    count += out.print(')');
  }
  return count;
}
}  // namespace ns1

namespace ns2 {
enum ESignedInt16 : int16_t { kMinusOne = -1 };
}

namespace ns3 {
enum class EScoped { kTwenty = 20 };
}

namespace ns4 {
enum class EScopedUnsignedInt8 : uint8_t { kOne = 1, kMax = 255 };
size_t PrintValueTo(EScopedUnsignedInt8 v, Print& out) {
  size_t count = 0;
  if (v == EScopedUnsignedInt8::kOne) {
    count += out.print("kOne");
  } else if (v == EScopedUnsignedInt8::kMax) {
    count += out.print("kMax");
  } else {
    count += out.print("Unknown: EScopedUnsignedInt8(");
    count += out.print(static_cast<int>(v), 10);
    count += out.print(')');
  }
  return count;
}

}  // namespace ns4

namespace ns5 {

struct SampleHasInsertInto {
  void InsertInto(mcucore::OPrintStream& strm) const {
    strm << 123 << mcucore::SetBase(3) << ' ' << 7;
  }
};

}  // namespace ns5

namespace o_print_stream_tests {
namespace {
using ::mcucore::BaseDec;
using ::mcucore::BaseHex;
using ::mcucore::BaseTwo;
using ::mcucore::OPrintStream;
using ::mcucore::SetBase;
using ::mcucore::test::PrintToStdString;
using ::mcucore::test::SampleHasPrintTo;
using ::mcucore::test::SamplePrintable;
using ::ns1::EScopeLess;
using ::ns2::ESignedInt16;
using ::ns3::EScoped;
using ::ns4::EScopedUnsignedInt8;

template <typename T>
void VerifyOPrintStream(const T value, std::string_view expected) {
  PrintToStdString p2ss;
  OPrintStream out(p2ss);
  out << value;
  EXPECT_EQ(p2ss.str(), expected) << "Value: " << value;
}

template <typename T, typename M>
void VerifyOPrintStreamManipulator(M manipulator, const T value,
                                   std::string_view expected) {
  PrintToStdString p2ss;
  OPrintStream out(p2ss);
  out << manipulator << value;
  EXPECT_EQ(p2ss.str(), expected) << "Value: " << value;
}

template <typename T>
void VerifyOPrintStreamBases(const T value, std::string_view decimal_expected,
                             std::string_view hexadecimal_expected,
                             std::string_view binary_expected,
                             std::string_view base36_expected,
                             std::string_view base8_expected) {
  VerifyOPrintStreamManipulator<T>(BaseDec, value, decimal_expected);
  VerifyOPrintStreamManipulator<T>(BaseHex, value, hexadecimal_expected);
  VerifyOPrintStreamManipulator<T>(BaseTwo, value, binary_expected);
  VerifyOPrintStreamManipulator<T>(SetBase(36), value, base36_expected);
  VerifyOPrintStreamManipulator<T>(SetBase(8), value, base8_expected);
}

template <typename T>
void VerifyOPrintStreamBaseless(const T value, std::string_view expected) {
  VerifyOPrintStreamManipulator<T>(BaseDec, value, expected);
  VerifyOPrintStreamManipulator<T>(BaseHex, value, expected);
  VerifyOPrintStreamManipulator<T>(BaseTwo, value, expected);
  VerifyOPrintStreamManipulator<T>(SetBase(36), value, expected);
  VerifyOPrintStreamManipulator<T>(SetBase(8), value, expected);
}

TEST(OPrintStreamTest, BuiltInTypes) {
  VerifyOPrintStreamBaseless<bool>(false, "false");
  VerifyOPrintStreamBaseless<bool>(true, "true");

  VerifyOPrintStreamBaseless<char>('a', "a");
  VerifyOPrintStreamBaseless<char>('\0', std::string_view("\0", 1));

  VerifyOPrintStreamBases<unsigned char>(0, "0", "0", "0", "0", "0");
  VerifyOPrintStreamBases<unsigned char>(1, "1", "0x1", "0b1", "1", "01");
  VerifyOPrintStreamBases<unsigned char>(255, "255", "0xFF", "0b11111111", "73",
                                         "0377");

  VerifyOPrintStreamBases<signed char>(-128, "-128", "-0x80", "-0b10000000",
                                       "-3K", "-0200");
  VerifyOPrintStreamBases<signed char>(-127, "-127", "-0x7F", "-0b1111111",
                                       "-3J", "-0177");
  VerifyOPrintStreamBases<signed char>(-1, "-1", "-0x1", "-0b1", "-1", "-01");
  VerifyOPrintStreamBases<signed char>(0, "0", "0", "0", "0", "0");
  VerifyOPrintStreamBases<signed char>(1, "1", "0x1", "0b1", "1", "01");
  VerifyOPrintStreamBases<signed char>(127, "127", "0x7F", "0b1111111", "3J",
                                       "0177");

  VerifyOPrintStreamBases<int16_t>(-32768, "-32768", "-0x8000",
                                   "-0b1000000000000000", "-PA8", "-0100000");
  VerifyOPrintStreamBases<int16_t>(-32767, "-32767", "-0x7FFF",
                                   "-0b111111111111111", "-PA7", "-077777");
  VerifyOPrintStreamBases<int16_t>(-1, "-1", "-0x1", "-0b1", "-1", "-01");
  VerifyOPrintStreamBases<int16_t>(0, "0", "0", "0", "0", "0");
  VerifyOPrintStreamBases<int16_t>(1, "1", "0x1", "0b1", "1", "01");
  VerifyOPrintStreamBases<int16_t>(32767, "32767", "0x7FFF",
                                   "0b111111111111111", "PA7", "077777");

  VerifyOPrintStreamBases<uint16_t>(0, "0", "0", "0", "0", "0");
  VerifyOPrintStreamBases<uint16_t>(1, "1", "0x1", "0b1", "1", "01");
  VerifyOPrintStreamBases<uint16_t>(65535, "65535", "0xFFFF",
                                    "0b1111111111111111", "1EKF", "0177777");

  VerifyOPrintStreamBases<int32_t>(-2147483648, "-2147483648", "-0x80000000",
                                   "-0b10000000000000000000000000000000",
                                   "-ZIK0ZK", "-020000000000");
  VerifyOPrintStreamBases<int32_t>(-1, "-1", "-0x1", "-0b1", "-1", "-01");
  VerifyOPrintStreamBases<int32_t>(0, "0", "0", "0", "0", "0");
  VerifyOPrintStreamBases<int32_t>(1, "1", "0x1", "0b1", "1", "01");
  VerifyOPrintStreamBases<int32_t>(2147483647, "2147483647", "0x7FFFFFFF",
                                   "0b1111111111111111111111111111111",
                                   "ZIK0ZJ", "017777777777");

  VerifyOPrintStreamBases<uint32_t>(0, "0", "0", "0", "0", "0");
  VerifyOPrintStreamBases<uint32_t>(1, "1", "0x1", "0b1", "1", "01");
  VerifyOPrintStreamBases<uint32_t>(4294967295, "4294967295", "0xFFFFFFFF",
                                    "0b11111111111111111111111111111111",
                                    "1Z141Z3", "037777777777");

  VerifyOPrintStreamBases<int64_t>(
      std::numeric_limits<int64_t>::min(), "-9223372036854775808",
      "-0x8000000000000000",
      "-0b1000000000000000000000000000000000000000000000000000000000000000",
      "-1Y2P0IJ32E8E8", "-01000000000000000000000");
  VerifyOPrintStreamBases<int64_t>(-1, "-1", "-0x1", "-0b1", "-1", "-01");
  VerifyOPrintStreamBases<int64_t>(0, "0", "0", "0", "0", "0");
  VerifyOPrintStreamBases<int64_t>(1, "1", "0x1", "0b1", "1", "01");
  VerifyOPrintStreamBases<int64_t>(
      std::numeric_limits<int64_t>::max(), "9223372036854775807",
      "0x7FFFFFFFFFFFFFFF",
      "0b111111111111111111111111111111111111111111111111111111111111111",
      "1Y2P0IJ32E8E7", "0777777777777777777777");

  VerifyOPrintStreamBases<uint64_t>(std::numeric_limits<uint64_t>::min(), "0",
                                    "0", "0", "0", "0");
  VerifyOPrintStreamBases<uint64_t>(1, "1", "0x1", "0b1", "1", "01");
  VerifyOPrintStreamBases<uint64_t>(
      std::numeric_limits<uint64_t>::max(), "18446744073709551615",
      "0xFFFFFFFFFFFFFFFF",
      "0b1111111111111111111111111111111111111111111111111111111111111111",
      "3W5E11264SGSF", "01777777777777777777777");

  // 2 digits to the right of the decimal point, unless more features are added
  // to OPrintStream to allow specifying these values, as std::basic_ostream
  // does via std::hex, etc.
  VerifyOPrintStreamBaseless<float>(-1, "-1.00");
  VerifyOPrintStreamBaseless<float>(0, "0.00");
  VerifyOPrintStreamBaseless<float>(0.99999, "1.00");
  VerifyOPrintStreamBaseless<double>(-1, "-1.00");
  VerifyOPrintStreamBaseless<double>(0, "0.00");
  VerifyOPrintStreamBaseless<double>(0.99999, "1.00");
}

TEST(OPrintStreamTest, FlashStrings) {
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << FLASHSTR("abc") << MCU_FLASHSTR("def") << MCU_PSD("ghi")
        << MCU_PSD("") << MCU_PSV("jkl")
        << mcucore::ProgmemString(FLASHSTR("mno"));
    EXPECT_EQ(p2ss.str(), "abcdefghijklmno");
  }
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << MCU_PSD_1024(TEST_STR_1023);
    EXPECT_EQ(p2ss.str(), TEST_STR_1023);
  }
}

TEST(OPrintStreamTest, StringLiteral) {
  PrintToStdString p2ss;
  OPrintStream out(p2ss);
  out << "abc";
  EXPECT_EQ(p2ss.str(), "abc");
}

TEST(OPrintStreamTest, Printable) {
  SamplePrintable value("abc");
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value;
    EXPECT_EQ(p2ss.str(), "abc");
  }
  {
    auto& value_ref = value;
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value_ref;
    EXPECT_EQ(p2ss.str(), "abc");
  }
}

TEST(OPrintStreamTest, ConstPrintable) {
  const SamplePrintable value("def");
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value;
    EXPECT_EQ(p2ss.str(), "def");
  }
  {
    auto& value_ref = value;
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value_ref;
    EXPECT_EQ(p2ss.str(), "def");
  }
}

TEST(OPrintStreamTest, HasPrintTo) {
  SampleHasPrintTo value("ghi");
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value;
    EXPECT_EQ(p2ss.str(), "ghi");
  }
  {
    auto& value_ref = value;
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value_ref;
    EXPECT_EQ(p2ss.str(), "ghi");
  }
}

TEST(OPrintStreamTest, StdStringHasPrintValueTo) {
  std::string value("ghi");
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value;
    EXPECT_EQ(p2ss.str(), "ghi");
  }
  {
    auto& value_ref = value;
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value_ref;
    EXPECT_EQ(p2ss.str(), "ghi");
  }
  {
    const auto& value_ref = value;
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value_ref;
    EXPECT_EQ(p2ss.str(), "ghi");
  }
}

TEST(OPrintStreamTest, ConstHasPrintTo) {
  const SampleHasPrintTo value("jkl");
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value;
    EXPECT_EQ(p2ss.str(), "jkl");
  }
  {
    auto& value_ref = value;
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value_ref;
    EXPECT_EQ(p2ss.str(), "jkl");
  }
}

TEST(OPrintStreamTest, ConstHasInsertInto) {
  const ns5::SampleHasInsertInto value;
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << 123 << ' ' << value << " " << 123;
    EXPECT_EQ(p2ss.str(), "123 123 21 123");
  }
  {
    auto& value_ref = value;
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << BaseHex << 123 << ' ' << value_ref << " " << 123;
    EXPECT_EQ(p2ss.str(), "0x7B 123 21 0x7B");
  }
}

TEST(OPrintStreamTest, ChangeBase) {
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);

    out << 127 << " " << BaseHex << 127 << ' ' << BaseTwo << 127;
    EXPECT_EQ(p2ss.str(), "127 0x7F 0b1111111");
  }
  // Print supports bases from 2 to 36.
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out.set_base(3);
    for (int i = 0; i <= 10; ++i) {
      out << ' ' << i;
    }
    EXPECT_EQ(p2ss.str(), " 0 1 2 10 11 12 20 21 22 100 101");
  }
}

TEST(OPrintStreamTest, ValidEnum) {
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EScopeLess::kSixteen << " " << BaseHex << EScopeLess::kSixteen << ' '
        << BaseTwo << EScopeLess::kSixteen;
    EXPECT_EQ(p2ss.str(), "kSixteen kSixteen kSixteen");
  }

  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << ESignedInt16::kMinusOne << " " << BaseHex << ESignedInt16::kMinusOne
        << ' ' << BaseTwo << ESignedInt16::kMinusOne;
    EXPECT_EQ(p2ss.str(), "-1 -0x1 -0b1");
  }

  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EScoped::kTwenty << " " << BaseHex << EScoped::kTwenty << ' '
        << BaseTwo << EScoped::kTwenty;
    EXPECT_EQ(p2ss.str(), "20 0x14 0b10100");
  }

  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EScopedUnsignedInt8::kOne << " " << EScopedUnsignedInt8::kMax;
    out << " " << BaseHex;
    out << EScopedUnsignedInt8::kOne << " " << EScopedUnsignedInt8::kMax;
    out << " " << BaseTwo;
    out << EScopedUnsignedInt8::kOne << " " << EScopedUnsignedInt8::kMax;
    EXPECT_EQ(p2ss.str(), "kOne kMax kOne kMax kOne kMax");
  }
}

TEST(OPrintStreamTest, BogusEnum) {
  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EScopeLess(0) << " " << BaseHex << EScopeLess(-99) << ' ' << BaseTwo
        << EScopeLess(127);
    EXPECT_EQ(p2ss.str(),
              "Unknown: EScopeLess(0) Unknown: EScopeLess(-99) Unknown: "
              "EScopeLess(127)");
  }

  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << ESignedInt16(-32768) << " " << BaseHex << ESignedInt16(-32768) << ' '
        << BaseTwo << ESignedInt16(-32768);
    EXPECT_EQ(p2ss.str(), "-32768 -0x8000 -0b1000000000000000");
  }

  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EScoped(127) << " " << BaseHex << EScoped(127) << ' ' << BaseTwo
        << EScoped(127);
    EXPECT_EQ(p2ss.str(), "127 0x7F 0b1111111");
  }

  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EScopedUnsignedInt8(127) << " " << BaseHex
        << EScopedUnsignedInt8(127) << ' ' << BaseTwo
        << EScopedUnsignedInt8(127);
    EXPECT_EQ(p2ss.str(),
              "Unknown: EScopedUnsignedInt8(127) Unknown: "
              "EScopedUnsignedInt8(127) Unknown: EScopedUnsignedInt8(127)");
  }
}

}  // namespace
}  // namespace o_print_stream_tests
