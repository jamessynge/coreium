#include "o_print_stream.h"

#include <stdint.h>

#include <string_view>

#include "extras/test_tools/print_to_std_string.h"
#include "extras/test_tools/sample_printable.h"
#include "gtest/gtest.h"

// DO NOT SUBMIT until a test for enums with PrintValueTo is added.

namespace mcucore {
namespace test {
namespace {

template <typename T>
void VerifyOPrintStream(const T value, std::string_view expected) {
  mcucore::test::PrintToStdString p2ss;
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
  mcucore::test::PrintToStdString p2ss;
  OPrintStream out(p2ss);
  out << "abc";
  EXPECT_EQ(p2ss.str(), "abc");
}

TEST(OPrintStreamTest, Printable) {
  mcucore::test::SamplePrintable value("abc");
  {
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value;
    EXPECT_EQ(p2ss.str(), "abc");
  }
  {
    auto& value_ref = value;
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value_ref;
    EXPECT_EQ(p2ss.str(), "abc");
  }
}

TEST(OPrintStreamTest, ConstPrintable) {
  const mcucore::test::SamplePrintable value("abc");
  {
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value;
    EXPECT_EQ(p2ss.str(), "abc");
  }
  {
    auto& value_ref = value;
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << value_ref;
    EXPECT_EQ(p2ss.str(), "abc");
  }
}

TEST(OPrintStreamTest, ChangeBase) {
  {
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);

    out << 127 << " " << BaseHex << 127 << ' ' << BaseTwo << 127;
    EXPECT_EQ(p2ss.str(), "127 0x7F 0b1111111");
  }
  // Print supports bases from 2 to 36.
  {
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out.set_base(3);
    for (int i = 0; i <= 10; ++i) {
      out << ' ' << i;
    }
    EXPECT_EQ(p2ss.str(), " 0 1 2 10 11 12 20 21 22 100 101");
  }
}

TEST(OPrintStreamTest, Enum) {
  // OPrintStream::set_base (as called by BaseHex) doesn't apply to enum as they
  // aren't matched by is_integral, so all the numbers are printed as the
  // decimal values that we converted to the enum type.
  {
    enum ScopeLess { kV = 0 };
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << ScopeLess(127) << " " << BaseHex << ScopeLess(127) << ' ' << BaseTwo
        << ScopeLess(127);
    EXPECT_EQ(p2ss.str(), "127 127 127");
  }
  {
    enum EUnderlying : int16_t { kV = -1 };
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);
    out << EUnderlying(127) << " " << BaseHex << EUnderlying(127) << ' '
        << BaseTwo << EUnderlying(127);
    EXPECT_EQ(p2ss.str(), "127 127 127");
  }

  {
    enum class Scoped { kV };
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);

    out << Scoped(127) << " " << BaseHex << Scoped(127) << ' ' << BaseTwo
        << Scoped(127);
    EXPECT_EQ(p2ss.str(), "127 127 127");
  }

  {
    enum class ScopedAndUnderlying { kV };
    mcucore::test::PrintToStdString p2ss;
    OPrintStream out(p2ss);

    out << ScopedAndUnderlying(127) << " " << BaseHex
        << ScopedAndUnderlying(127) << ' ' << BaseTwo
        << ScopedAndUnderlying(127);
    EXPECT_EQ(p2ss.str(), "127 127 127");
  }
}

}  // namespace
}  // namespace test
}  // namespace mcucore
