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
#include <string>  // pragma: keep standard include
#endif

namespace mcucore {

// Most methods of this class call other methods based on the type of their
// template parameters. Since the type is fully determined at compile time, it
// should be that the compiler is able to replace most such functions calls with
// a direct call to the leafmost function call; given the following example:
//
// [1]    SomeType foo;
// [2]    OPrintStream ostrm(Serial);
// [3]    ostrm << &foo;
//
// the insertion operator (operator<<) ultimately calls the do_print_d overload
// for values of type "const void*". The compiler should thus be able to replace
// line [3] with (roughly) this:
//
//        ostrm.print_hex(reinterpret_cast<const uintptr_t>(&foo));
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
  // enabling overloading for concrete T's. TBD whether that is necessary.
  template <typename T>
  inline OPrintStream& operator<<(const T value) {
    // Choose the overload of do_print_a based on whether T is a class or struct
    // with a printTo method.
    do_print_a(value, has_print_to<T>{});
    return *this;
  }

  // Set the base in which numbers are printed.
  inline void set_base(uint8_t base) { base_ = base; }

 protected:
  // Exposed so that subclasses can call this.
  template <typename T>
  inline void printValue(const T value) {
    do_print_a(value, has_print_to<T>{});
  }

  Print& out_;
  uint8_t base_;

 private:
  // I'm using type traits to steer the call to the appropriate method or
  // function for printing a value of type T.

  // T is a class with a printTo function.
  template <typename T>
  inline void do_print_a(const T value, true_type /*has_print_to*/) {
    value.printTo(out_);
  }

  // Type T does NOT have a printTo method.
  template <typename T>
  inline void do_print_a(const T value, false_type /*!has_print_to*/) {
    // Choose the overload of do_print_b based on whether T is an int or similar
    // type.
    do_print_b(value, is_integral<T>{});
  }

  // Type T is an integral type (includes bool and char).
  // TODO(jamessynge): Consider whether making OPrintStream::operator<< a global
  // function, rather than a member function, would also allow us to change
  // BaseHex, such that the return type for streaming BaseHex would be
  // OPrintStreamHex, a sub-class of OPrintStream. That would avoid the need for
  // OPrintStream to have a base_ member, and thus the method below for printing
  // integral types would be simpler, which in turn might mean reduced
  // complexity in the generated code (i.e. it would be closer to just calling
  // Print::print(value, HEX)).
  template <typename T>
  void do_print_b(const T value, true_type /*is_integral*/) {
    if (base_ == 10) {
      out_.print(value, base_);
    } else if (base_ == 16) {
      print_hex(value);
    } else if (base_ == 2) {
      out_.print('0');
      out_.print('b');
      out_.print(value, base_);
    } else {
      out_.print(value, base_);
    }
  }

  // Type T is NOT an integral type.
  template <typename T>
  inline void do_print_b(const T value, false_type /*!is_integral*/) {
    // Choose the overload of do_print_c based on whether there exists a
    // PrintValueTo function with this signature:
    //     PrintValueTo(T, Print&)
    do_print_c(value, has_print_value_to<T>{});
  }

  template <typename T>
  inline void do_print_c(const T value, true_type /*has_print_value_to*/) {
    PrintValueTo(value, out_);
  }

  template <typename T>
  inline void do_print_c(const T value, false_type /*has_print_value_to*/) {
    // Choose the overload of do_print_d based on whether T is a pointer.
    do_print_d(value, is_pointer<T>{});
  }

  inline void do_print_d(const char* value, true_type /*is_pointer*/) {
    out_.print(value);
  }

  inline void do_print_d(const __FlashStringHelper* value,
                         true_type /*is_pointer*/) {
    out_.print(value);
  }

  inline void do_print_d(OPrintStreamManipulator manipulator,
                         true_type /*is_pointer*/) {
    (*manipulator)(*this);
  }

  // Print any pointer of a type we don't directly support  of an otherwise
  inline void do_print_d(const void* misc_pointer, true_type /*is_pointer*/) {
    auto i = reinterpret_cast<const uintptr_t>(misc_pointer);
    print_hex(i);
  }

#if MCU_HOST_TARGET
  void do_print_d(const std::string& value, false_type /*!is_pointer*/) {
    out_.write(value.data(), value.size());
  }
#endif

  // T is not a pointer. Just pass the value to to print. Most likely T is a
  // floating point type.
  template <typename T>
  void do_print_d(const T value, false_type /*!is_pointer*/) {
    out_.print(value);
  }

  template <typename T>
  void print_hex(const T value) {
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

#endif  // MCUCORE_SRC_O_PRINT_STREAM_H_
