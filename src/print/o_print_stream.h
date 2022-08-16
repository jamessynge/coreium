#ifndef MCUCORE_SRC_PRINT_O_PRINT_STREAM_H_
#define MCUCORE_SRC_PRINT_O_PRINT_STREAM_H_

// Support for streaming into a Print instance. While I've used this primarily
// in the context of logging.h, OPrintStream can also make using Serial much
// easier. For example, making use of mcucore::LogSink:
//
//    #include "log_sink.h"
//    void setup() {
//      const int kBaudRate = 9600;
//      Serial.begin(kBaudRate);
//      while (!Serial) {}
//      mcucore::LogSink()
//          << FLASHSTR("Starting the program, with baud rate ") << kBaudRate
//          << FLASHSTR(", current time: ") << millis();
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
// (We use FLASHSTR(str) rather than F(str) due to conflicts caused by Arduino's
// F as the name for that macro. While macros should generally be avoided, they
// are nonetheless useful, so when they are defined they should have names that
// are not very likely to collide with choices made by others. Sigh.)
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "misc/to_unsigned.h"
#include "print/has_insert_into.h"
#include "print/has_print_to.h"
#include "print/print_misc.h"
#include "semistd/limits.h"
#include "semistd/type_traits.h"
#include "strings/has_progmem_char_array.h"
#include "strings/progmem_string_data.h"

#if MCU_HOST_TARGET
#include <string>       // pragma: keep standard include
#include <string_view>  // pragma: keep standard include
#endif

namespace mcucore {

class OPrintStream;

// Set the base for printing numbers to 16. For example:
//
//   strm << "In hex: " << BaseHex << 123 << ' ' << -1;
//
// Will insert "In hex: 0x7B -0x1" into strm.
void BaseHex(OPrintStream& strm);

// Set the base for printing numbers to 10 (the default). For example:
//
//   strm << BaseHex << "Value: " << 123 << BaseDec << ", " << 123;
//
// Will insert "Value: 0x7B, 123" into strm.
void BaseDec(OPrintStream& strm);

// Set the base for printing numbers to 2. For example:
//
//   strm << BaseTwo << "Value: " << 10 << BaseHex << ", " << 10;
//
// Will insert "Value: 0b1010, 0xA" into strm.
void BaseTwo(OPrintStream& strm);

// Set the base to a value in the range [2, 36].
//
//   strm << SetBase(4) << 5;
//
// Will insert "11" into strm. There is no standard prefix, such as "0x" or
// "0b", for bases other than 2, 8 and 16.
struct SetBase {
  explicit SetBase(uint8_t base) : base(base) {}
  const uint8_t base;
};

// The template function PrintValue is the core of OPrintStream, and the
// specific version that is realized is selected based on the use of the
// enable_if_t<T> type trait. Care has been taken to ensure that only a single
// PrintValue definition works for each type that we intend to support.
//
// Since the type of the value to be printed is known at compile time, the
// compiler *should* be able to inline most calls to the appropriate function
// calls defined outside of OPrintStream (e.g. Foo::printTo(Print&) or
// PrintValueTo(const Foo&, Print&)). In this example:
//
// [1]    SomeType foo;
// [2]    OPrintStream ostrm(Serial);
// [3]    ostrm << &foo;
//
// The insertion operator (operator<<) ultimately calls the PrintValue
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
  OPrintStream(const OPrintStream& out) = default;
  OPrintStream(OPrintStream&& out) = default;

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
  // enable_if_t used to select one of them based on the type of the value.
  // Ideally the compiled code is as specific to the inserted type as possible
  // so that most stream insertions require as little code as possible.
  //
  // NOTE: The value (10, 20, etc. below) assigned to the type produced by
  // enable_if_t is used to address the problem described as follows by
  // https://en.cppreference.com/w/cpp/types/enable_if:
  //
  //   A common mistake is to declare two function templates that differ only in
  //   their default template arguments. This does not work because the
  //   declarations are treated as redeclarations of the same function template
  //   (default template arguments are not accounted for in function template
  //   equivalence).
  //
  // Eventually I want to use a priority tag to force the order of matching,
  // which should eliminate the need to use enable_if<!X> to avoid conflicting
  // matches that arent' desired.

  // The value is of type char, but not signed char or unsigned char. We want
  // single characters (e.g. '\n') to be streamed as characters, but want small
  // numbers represented in bytes (e.g. uint8_t(10) or int8_t(-1)) to be
  // formatted as integers, i.e. handled by the Integer case below.
  template <typename Char, enable_if_t<is_char<Char>::value, int> = 10>
  inline void PrintValue(const Char char_value) {
    out_.print(char_value);
  }

  // The value is of a class type for which the class has an InsertInto
  // function. The function is in charge of its own base for numbers, and
  // anything else that we may allow to be changed in the OPrintStream, so we
  // pass the function an OPrintStream with the same Print instance.
  template <typename HasInsertInto,
            enable_if_t<has_insert_into<HasInsertInto>::value, int> = 20>
  inline void PrintValue(const HasInsertInto& value_has_insert_into) {
    OPrintStream strm(out_);
    value_has_insert_into.InsertInto(strm);
  }

  // Reserving 30 for HasInsertValueInto.

  // The value's type has a progmem_char_array member function, i.e. it is a
  // ProgmemStringData.
  template <typename PSD,
            enable_if_t<has_progmem_char_array<PSD>::value, int> = 40>
  inline void PrintValue(const PSD str) {
    PrintProgmemStringData(str, out_);
  }

  // The value is of a class type for which the class has a printTo function.
  template <typename HasPrintTo,
            enable_if_t<has_print_to<HasPrintTo>::value, int> = 50>
  inline void PrintValue(const HasPrintTo value_has_print_to) {
    value_has_print_to.printTo(out_);
  }

  // The value is of a type for which there is a PrintValueTo(T, Print&)
  // function.
  template <typename HasPrintValueTo,
            enable_if_t<has_print_value_to<HasPrintValueTo>::value &&
                            !is_char<HasPrintValueTo>::value,
                        int> = 60>
  inline void PrintValue(const HasPrintValueTo can_print_value_to) {
    PrintValueTo(can_print_value_to, out_);
  }

  // The value is a bool and there is no PrintValueTo function defined for bool.
  template <typename Bool, enable_if_t<is_same<Bool, bool>::value &&
                                           !has_print_value_to<Bool>::value,
                                       int> = 70>
  inline void PrintValue(const Bool value) {
    out_.print(value ? MCU_FLASHSTR("true") : MCU_FLASHSTR("false"));
  }

  // The value is of an integral type (excluding bool and char), but is not of a
  // type for which there is a PrintValueTo function defined. char is excluded
  // so that it can be used for for characters rather than numbers, and bool is
  // excluded so that we can print such values as true or false.
  template <
      typename Integer,
      enable_if_t<is_integral<Integer>::value && !is_char<Integer>::value &&
                      !is_same<Integer, bool>::value &&
                      !has_print_value_to<Integer>::value,
                  int> = 80>
  inline void PrintValue(const Integer integer_value) {
    PrintInteger(integer_value);
  }

  // The value is a floating point number, for which there is not a PrintValueTo
  // function.
  template <typename Float, enable_if_t<is_floating_point<Float>::value &&
                                            !has_print_value_to<Float>::value,
                                        int> = 90>
  inline void PrintValue(const Float fp_value) {
    out_.print(fp_value);
  }

  // The value is a pointer for which there is NOT a PrintValueTo function.
  template <typename Pointer,
            enable_if_t<is_pointer<Pointer>::value &&
                            !has_print_value_to<Pointer>::value,
                        int> = 100>
  inline void PrintValue(const Pointer pointer_value) {
    PrintPointer(pointer_value);
  }

  // The value is an enumerator, for which there is not a PrintValueTo function.
  // Print it as an integer of the underlying type.
  template <typename Enum, enable_if_t<is_enum<Enum>::value &&
                                           !has_print_value_to<Enum>::value,
                                       int> = 110>
  inline void PrintValue(const Enum value) {
    PrintInteger(static_cast<underlying_type_t<Enum>>(value));
  }

  // Finally a fallback for other types, for which there must be a
  // Print::print(Others) method.
  template <
      typename Others,
      enable_if_t<
          !is_char<Others>::value && !has_insert_into<Others>::value &&
              !has_print_to<Others>::value && !is_pointer<Others>::value &&
              !is_integral<Others>::value &&
              !is_floating_point<Others>::value &&
              !has_print_value_to<Others>::value &&
              !has_progmem_char_array<Others>::value && !is_enum<Others>::value,
          int> = 200>
  inline void PrintValue(const Others other_type_value) {
    out_.print(other_type_value);
  }

  void PrintValue(SetBase set_base) { base_ = set_base.base; }

  //////////////////////////////////////////////////////////////////////////////
  // Handle various types of pointers.

  inline void PrintPointer(const char* value) {
    if (value != nullptr) {
      out_.print(value);
    }
  }

  inline void PrintPointer(const __FlashStringHelper* value) {
    if (value != nullptr) {
      out_.print(value);
    }
  }

  inline void PrintPointer(OPrintStreamManipulator manipulator) {
    // Can't say this here: MCU_DCHECK_NE(manipulator, nullptr);
    // because it would introduce a cycle.
    (*manipulator)(*this);
  }

  // Print any pointer of a type we don't directly support as a hexidecimal
  // number (i.e. the address of the referenced thing).
  inline void PrintPointer(const void* misc_pointer) {
    auto i = reinterpret_cast<const uintptr_t>(misc_pointer);
    PrintHex(i);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Print integers.

  template <typename T>
  void PrintInteger(T value) {
    // Print.print(T value, int base) can print integer values of type T in any
    // base in the range [2, 36]; if the base is outside of that range, it
    // prints in base 10. Note that we don't include the base prefix if the
    // value is zero, which is matches the behavior of std::basic_ostream.
    if (value == 0 || base_ == 10 || base_ < 2 || base_ > 36) {
      out_.print(value, 10);
    } else {
      using UT = decltype(ToUnsigned(value));
      UT unsigned_value;
      if (value >= 0) {
        unsigned_value = value;
      } else {
        // Print the negative sign before the value because Arduino's Print
        // class won't do so for signed values when printing with a base other
        // than 10.
        out_.print('-');

        // Implement (approximately) `UT unsigned_value = abs(value)`. We assume
        // here that signed integers are represented with twos complement
        // values, and thus that abs(numeric_limits<T>::min()) can't be
        // represented as a T.
        unsigned_value = (numeric_limits<UT>::max() - value) + 1;
      }

      // There are standard prefixes for base-2, base-8 and base-16, but not
      // for the other bases.
      if (base_ == 16) {
        // We treat base-16 separately so that we can share the PrintHex
        // implementation with PrintPointer.
        PrintHex(unsigned_value);
      } else {
        if (base_ == 2) {
          out_.print('0');
          out_.print('b');
        } else if (base_ == 8) {
          out_.print('0');
        }
        out_.print(unsigned_value, base_);
      }
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

}  // namespace mcucore

#if MCU_HOST_TARGET
// These are defined outside of namespace mcucore so that C++ Argument Dependent
// Lookup will find these functions.
inline size_t PrintValueTo(const std::string& value, Print& out) {
  return out.write(value.data(), value.size());
}
inline size_t PrintValueTo(const std::string_view value, Print& out) {
  return out.write(value.data(), value.size());
}
#endif

#endif  // MCUCORE_SRC_PRINT_O_PRINT_STREAM_H_
