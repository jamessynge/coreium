#ifndef MCUCORE_SRC_O_PRINT_STREAM_H_
#define MCUCORE_SRC_O_PRINT_STREAM_H_

// Support for streaming into a Print instance. While I've used this primarily
// in the context of logging.h, OPrintStream can also make using Serial much
// easier. For example, making use of mcucore::LogSink:
//
//    #include "log_sink.h"
//    void setup() {
//      const int kBaudRate = 9600;
//      Serial.begin(kBaudRate);
//      while (!Serial) {}
//      mcucore::LogSink() << F("Starting the program, with baud rate ")
//                         << kBaudRate << F(", current time: ") << millis();
//    }
//
// Here LogSink is taking care of writing a newline at the end, and flushing the
// Serial buffers (if there are any). If we used Serial directly, that last
// statement would become these 4:
//
//      Serial.print(F("Starting the program, with baud rate "));
//      Serial.print(kBaudRate);
//      Serial.print(F(", current time: "));
//      Serial.printLn(millis());
//
// Author: james.synge@gmail.com

#include "has_print_to.h"
#include "mcucore_platform.h"
#include "type_traits.h"

#if MCU_HOST_TARGET
#include <string>       // pragma: keep standard include
#include <string_view>  // pragma: keep standard include
#endif

namespace mcucore {

// The template function PrintValue is the core of this class, and the specific
// version that is realized is selected based on the use of the enable_if<T>
// type trait. Care has been taken to ensure that only a single PrintValue
// definition works for each type that we intend to support.
//
// Since the type of the value to be printed is known at compile time, it should
// be that the compiler is able to inline most calls to the appropriate function
// calls defined outside of this class (e.g. Foo::printTo(Print&) or
// PrintValueTo(const Foo&, Print&)).
//
// [1]    SomeType foo;
// [2]    OPrintStream ostrm(Serial);
// [3]    ostrm << &foo;
//
// the insertion operator (operator<<) ultimately calls the PrintValue
// definition for pointers, which in turn calls the PrintPointer overload for
// values of type "const void*". The compiler should thus be able to replace
// line [3] with (roughly) this:
//
//        ostrm.PrintHex(reinterpret_cast<const uintptr_t>(&foo));
//
// While it isn't necessary to declare the member functions as inline, I'm
// choosing to do so to clarify to readers that I expect these functions to be
// inlined in practice (at least with appropriate optimizations enabled).
class OPrintStream {
 public:
  // An OPrintStreamManipulator is a function which can modify the OPrintStream
  // instance. So far the only three are BaseTwo, BaseDec and BaseHex, allowing
  // a logging statement to choose the base for printing numbers.
  using OPrintStreamManipulator = void (*)(OPrintStream&);

  explicit OPrintStream(Print& out) : out_(out), base_(10) {}
  OPrintStream() : out_(::Serial), base_(10) {}

  // It may be that this should be a friend function rather than a method, thus
  // enabling overloading for concrete T's. TBD whether that is necessary. I've
  // instead provided for PrintValueTo as the means to enable this.
  template <typename T>
  inline OPrintStream& operator<<(const T value) {
    PrintValue(value);
    return *this;
  }

  // Set the base in which numbers are printed.
  inline void set_base(uint8_t base) { base_ = base; }

 protected:
  // The template function DoPrint is exposed so that subclasses can call it,
  // rather than using an awkward call to the insertion operator.
  template <typename T>
  inline void DoPrint(const T value) {
    PrintValue(value);
  }

  Print& out_;
  uint8_t base_;

 private:
  template <typename T>
  using is_char =
      typename ::mcucore::is_same<char, typename ::mcucore::remove_cv<T>::type>;

  // We provide multiple definitions of the template function PrintValue, with
  // enable_if used to select one of them based on the type of the value.
  // Ideally the compiled code is as specific to the inserted type as possible
  // so that most stream insertions require as little code as possible.

  // The value is of type char, but not signed char or unsigned char. We want
  // single characters (e.g. '\n') to be streamed as characters, but want small
  // numbers represented in bytes (e.g. uint8_t(10) or int8_t(-1)) to be
  // formatted as integers, i.e. handled by the Integer case below.
  template <typename Char, enable_if_t<is_char<Char>::value, int> = 1>
  inline void PrintValue(const Char value) {
    out_.print(value);
  }

  // The value is of a class type for which the class has a printTo function.
  template <typename HasPrintTo,
            enable_if_t<has_print_to<HasPrintTo>::value, int> = 2>
  inline void PrintValue(const HasPrintTo value) {
    value.printTo(out_);
  }

  // The value is of a type for which there is a PrintValueTo(T, Print&)
  // function.
  template <typename HasPrintValueTo,
            enable_if_t<has_print_value_to<HasPrintValueTo>::value &&
                            !is_char<HasPrintValueTo>::value,
                        int> = 3>
  inline void PrintValue(const HasPrintValueTo value) {
    PrintValueTo(value, out_);
  }

  // The value is of an integral type (including bool), but is not a char, nor
  // of a type for which there is a PrintValueTo function defined.
  template <
      typename Integer,
      enable_if_t<is_integral<Integer>::value && !is_char<Integer>::value &&
                      !has_print_value_to<Integer>::value,
                  int> = 4>
  inline void PrintValue(const Integer value) {
    PrintInteger(value);
  }

  // The value is a floating point number, for which there is not a PrintValueTo
  // function.
  template <typename Float, enable_if_t<is_floating_point<Float>::value &&
                                            !has_print_value_to<Float>::value,
                                        int> = 4>
  inline void PrintValue(const Float value) {
    out_.print(value);
  }

  // The value is a pointer for which there is NOT a PrintValueTo function.
  template <typename Pointer,
            enable_if_t<is_pointer<Pointer>::value &&
                            !has_print_value_to<Pointer>::value,
                        int> = 5>
  inline void PrintValue(const Pointer value) {
    PrintPointer(value);
  }

  // The value is an enumerator, for which there is not a PrintValueTo function.
  // Print it as an integer of the underlying type.
  template <typename Enum, enable_if_t<is_enum<Enum>::value &&
                                           !has_print_value_to<Enum>::value,
                                       int> = 6>
  inline void PrintValue(const Enum value) {
    PrintInteger(static_cast<underlying_type_t<Enum>>(value));
  }

  // Finally a fallback for other types, for which there must be a
  // Print::print(Others) method.
  template <
      typename Others,
      enable_if_t<!has_print_to<Others>::value && !is_pointer<Others>::value &&
                      !is_integral<Others>::value &&
                      !is_floating_point<Others>::value &&
                      !has_print_value_to<Others>::value &&
                      !is_enum<Others>::value,
                  int> = 7>
  inline void PrintValue(const Others value) {
    out_.print(value);
  }

  //////////////////////////////////////////////////////////////////////////////

  inline void PrintPointer(const char* value) { out_.print(value); }

  inline void PrintPointer(const __FlashStringHelper* value) {
    out_.print(value);
  }

  inline void PrintPointer(OPrintStreamManipulator manipulator) {
    (*manipulator)(*this);
  }

  // Print any pointer of a type we don't directly support as a hexidecimal
  // number (i.e. the address of the referenced thing).
  inline void PrintPointer(const void* misc_pointer) {
    auto i = reinterpret_cast<const uintptr_t>(misc_pointer);
    PrintHex(i);
  }

  //////////////////////////////////////////////////////////////////////////////

  template <typename T, enable_if_t<sizeof(T) == sizeof(uint8_t), int> = 1>
  uint8_t ToUnsigned(T value) {
    return static_cast<uint8_t>(value);
  }

  template <typename T, enable_if_t<sizeof(T) == sizeof(uint16_t), int> = 2>
  uint16_t ToUnsigned(T value) {
    return static_cast<uint16_t>(value);
  }

  template <typename T, enable_if_t<sizeof(T) == sizeof(uint32_t), int> = 3>
  uint32_t ToUnsigned(T value) {
    return static_cast<uint32_t>(value);
  }

  template <typename T, enable_if_t<sizeof(T) == sizeof(uint64_t), int> = 4>
  uint64_t ToUnsigned(T value) {
    return static_cast<uint64_t>(value);
  }

  // Print an integer.
  template <typename T>
  void PrintInteger(const T value) {
    // Print.print(T value, int base) can print integer values of type T in any
    // base in the range [2, 36]; if the base is outside of that range, it
    // prints in base 10. Only base 10 is printed in a signed fashion; all other
    // bases are treated as unsigned T.
    if (base_ == 10 || base_ < 2 || base_ > 36) {
      out_.print(value, 10);
    } else if (base_ == 16) {
      PrintHex(ToUnsigned(value));
    } else if (base_ == 2) {
      out_.print('0');
      out_.print('b');
      out_.print(ToUnsigned(value), base_);
    } else {
      out_.print(ToUnsigned(value), base_);
    }
  }

  template <typename T>
  void PrintHex(const T value) {
    out_.print('0');
    out_.print('x');
    out_.print(value, 16);
  }
};

// Set the base for printing numbers to 16. For example:
//
//   strm << "In hex: " << BaseHex << 123;
//
// Will insert "In hex: 0x7B" into strm.
inline void BaseHex(OPrintStream& strm) { strm.set_base(16); }

// Set the base for printing numbers to 10 (the default). For example:
//
//   strm << BaseHex << "Value: " << 123 << BaseDec << ", " << 123;
//
// Will insert "Value: 0x7B, 123" into strm.
inline void BaseDec(OPrintStream& strm) { strm.set_base(10); }

// Set the base for printing numbers to 2. For example:
//
//   strm << BaseTwo << "Value: " << 10 << BaseHex << ", " << 10;
//
// Will insert "Value: 0b1010, 0xA" into strm.
inline void BaseTwo(OPrintStream& strm) { strm.set_base(2); }

#if MCU_HOST_TARGET
inline size_t PrintValueTo(const std::string& value, Print& out) {
  return out.write(value.data(), value.size());
}
inline size_t PrintValueTo(const std::string_view value, Print& out) {
  return out.write(value.data(), value.size());
}
#endif

}  // namespace mcucore

#endif  // MCUCORE_SRC_O_PRINT_STREAM_H_
