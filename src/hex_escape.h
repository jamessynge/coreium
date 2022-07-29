#ifndef MCUCORE_SRC_HEX_ESCAPE_H_
#define MCUCORE_SRC_HEX_ESCAPE_H_

// Support for printing strings (Printable's or similar) with non-printable
// ASCII characters hex escaped. Intended to produce output that is valid as a
// C/C++ string literal.
//
// Author: james.synge@gmail.com

#include "counting_print.h"
#include "has_print_to.h"
#include "mcucore_platform.h"
#include "o_print_stream.h"
#include "type_traits.h"

namespace mcucore {

// Nibble to uppercase ASCII hex character. Value of v must be in range 0
// through 15).
char NibbleToAsciiHex(uint8_t v);

enum class EHexEscapingState : uint8_t {
  kNormal = 0,
  // Last character output was the last character output for a hex escaped
  // character.
  kHexDigitOutput,
  // Last character output was a question mark. We need to know this IFF
  // avoiding printing trigraph sequences for ASCII characters outside of the
  // ISO/IEC 646 Invariant character set, i.e. the characters #, $, @, [, \, ],
  // ^, `, {, |, }, and ~.
  kQuestionMarkOutput
};

// Print |c| hex escaped to |out|. Actually backslash escapes backslash and
// double quote, and uses named escapes for newline (\n), carriage return (\t),
// etc.; for other control characters and non-ascii characters, uses a hex
// escape (e.g. \x01 or \xff).
size_t PrintCharHexEscaped(Print& out, char c);

// As above, but may escape the character also if the last output character was
// not kNormal. For example, if |c| is a hex digit (0-9, A-F, a-f), and state is
// kHexDigitOutput, then |c| must be escaped. |state| is updated accordingly.
// This function is useful for hex escaping multiple characters, e.g. for
// implementing PrintHexEscaped.
size_t PrintCharWithStateHexEscaped(Print& out, char c,
                                    EHexEscapingState& state);

// Wraps a Print instance, forwards output to that instance with hex escaping
// applied. Note that this does NOT add double quotes before and after the
// output.
class PrintHexEscaped : public Print {
 public:
  explicit PrintHexEscaped(Print& wrapped);

  // These are the two abstract virtual methods in Arduino's Print class. I'm
  // presuming that the uint8_t 'b' is actually an ASCII char.
  size_t write(uint8_t b) override;
  size_t write(const uint8_t* buffer, size_t size) override;

  // Export the other write methods.
  using Print::write;

 private:
  Print& wrapped_;
  EHexEscapingState state_;
};

template <class T>
class HexEscapedPrintable : public Printable {
 public:
  explicit HexEscapedPrintable(const T& wrapped) : wrapped_(wrapped) {}

  size_t printTo(Print& raw_out) const override {
    size_t count = raw_out.print('"');
    PrintHexEscaped out(raw_out);
    count += wrapped_.printTo(out);
    count += raw_out.print('"');
    return count;
  }

 private:
  const T& wrapped_;
};

template <typename HasPrintTo,
          enable_if_t<has_print_to<HasPrintTo>::value, int> = 0>
inline HexEscapedPrintable<HasPrintTo> HexEscaped(const HasPrintTo& v) {
  return HexEscapedPrintable<HasPrintTo>(v);
}

template <class T>
class HexEscapedViaOPrintStream : public Printable {
 public:
  explicit HexEscapedViaOPrintStream(T wrapped) : wrapped_(wrapped) {}

  size_t printTo(Print& raw_out) const override {
    CountingPrint counting_print(raw_out);
    counting_print.print('"');
    {
      PrintHexEscaped print_hex_escaped(counting_print);
      OPrintStream strm(print_hex_escaped);
      strm << wrapped_;
    }
    counting_print.print('"');
    return counting_print.count();
  }

 private:
  const T wrapped_;
};

template <typename T,
          enable_if_t<!has_print_to<T>::value && is_union_or_class<T>::value,
                      int> = 1>
inline HexEscapedViaOPrintStream<const T&> HexEscaped(const T& t) {
  return HexEscapedViaOPrintStream<const T&>(t);
}

template <typename T,
          enable_if_t<!has_print_to<T>::value && !is_union_or_class<T>::value,
                      int> = 1>
inline HexEscapedViaOPrintStream<T> HexEscaped(const T& t) {
  return HexEscapedViaOPrintStream<T>(t);
}

// Print a sequence of bytes in the standard (IEEE 802) format for printing
// Ethernet (EUI-48) addresses, i.e. as groups of 2 hexidecimal digits separated
// by a hyphen.
size_t PrintWithEthernetFormatting(Print& out, const uint8_t* ptr,
                                   uint8_t num_bytes);
template <uint8_t N>
size_t PrintWithEthernetFormatting(Print& out, const uint8_t (&mac_bytes)[N]) {
  return PrintWithEthernetFormatting(out, mac_bytes, N);
}

}  // namespace mcucore

#endif  // MCUCORE_SRC_HEX_ESCAPE_H_
