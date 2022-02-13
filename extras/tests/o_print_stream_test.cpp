#include "o_print_stream.h"

#include <stdint.h>

#include <string_view>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/sample_printable.h"
#include "gtest/gtest.h"

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

namespace o_print_stream_tests {
namespace {
using ::mcucore::BaseHex;
using ::mcucore::BaseTwo;
using ::mcucore::OPrintStream;
using ::mcucore::test::PrintToStdString;
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

TEST(OPrintStreamTest, BuiltInTypes) {
  VerifyOPrintStream<char>('a', "a");
  VerifyOPrintStream<char>('\0', std::string_view("\0", 1));

  VerifyOPrintStream<unsigned char>(0, "0");
  VerifyOPrintStream<unsigned char>(255, "255");

  VerifyOPrintStream<int16_t>(-32768, "-32768");
  VerifyOPrintStream<int16_t>(0, "0");
  VerifyOPrintStream<int16_t>(32767, "32767");

  VerifyOPrintStream<uint16_t>(0, "0");
  VerifyOPrintStream<uint16_t>(65535, "65535");

  VerifyOPrintStream<int32_t>(-2147483648, "-2147483648");
  VerifyOPrintStream<int32_t>(0, "0");
  VerifyOPrintStream<int32_t>(2147483647, "2147483647");

  VerifyOPrintStream<uint32_t>(0, "0");
  VerifyOPrintStream<uint32_t>(4294967295, "4294967295");

  VerifyOPrintStream<int64_t>(0, "0");
  VerifyOPrintStream<uint64_t>(0, "0");

  // 2 digits to the right of the decimal point, unless more features are added
  // to OPrintStream to allow specifying these values, as std::basic_ostream
  // does via std::hex, etc.
  VerifyOPrintStream<float>(-1, "-1.00");
  VerifyOPrintStream<float>(0, "0.00");
  VerifyOPrintStream<float>(0.99999, "1.00");
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
  const SamplePrintable value("abc");
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
  // OPrintStream::set_base (as called by BaseHex) doesn't apply to enum as they
  // aren't matched by is_integral, so all the numbers are printed as the
  // decimal values that we converted to the enum type, unless there is a
  // PrintValueTo function that takes over.
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
    EXPECT_EQ(p2ss.str(), "-1 -1 -1");
  }

  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EScoped::kTwenty << " " << BaseHex << EScoped::kTwenty << ' '
        << BaseTwo << EScoped::kTwenty;
    EXPECT_EQ(p2ss.str(), "20 20 20");
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
  // OPrintStream::set_base (as called by BaseHex) doesn't apply to enum as they
  // aren't matched by is_integral, so all the numbers are printed as the
  // decimal values that we converted to the enum type, unless there is a
  // PrintValueTo function that takes over.
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
    EXPECT_EQ(p2ss.str(), "-32768 -32768 -32768");
  }

  {
    PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EScoped(127) << " " << BaseHex << EScoped(127) << ' ' << BaseTwo
        << EScoped(127);
    EXPECT_EQ(p2ss.str(), "127 127 127");
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
