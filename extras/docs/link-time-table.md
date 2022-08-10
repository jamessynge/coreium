# Creating a Table of T in Flash

It can be useful to build various kinds of tables (arrays of some element type
T) during the build process. For example:

*   Enum to Name: e.g. for mapping to an HTTP status message or field name, or
    for debug strings.
*   Device Info: e.g. for list of devices that the program is configured to
    access; the elements can be structs with multiple fields.
*   Function Table; e.g. for a dispatch table, or for the list of functions to
    be called when a particular event occurs.

If the elements are all in the same C or C++ source file, this is fairly easy
using the PROGMEM *attribute*; see
[Data in Program Space](https://www.nongnu.org/avr-libc/user-manual/pgmspace.html).

However, if they are elements are in multiple source files, then we need to make
use of linker features to combine the elements into a single table. For more,
see:

*   [The GNU Linker](https://www.eecs.umich.edu/courses/eecs373/readings/Linker.pdf)
    *   Section 3.5.4, Source Code References
    *   Section 3.10.9, Builtin Functions
*   [How to implement a compile-time [dispatch] table for AVR?](https://stackoverflow.com/q/39733931)
*   [Implementing a compile-time read-only function pointer table in GCC](https://stackoverflow.com/q/19845635)
*   [The most thoroughly commented linker script](https://blog.thea.codes/the-most-thoroughly-commented-linker-script/)

## Testing

My particular interest is in on-device testing of McuCore, McuNet and
TinyAlpacaServer. There are various libraries for doing this (e.g.
[AUnit](https://github.com/bxparks/AUnit)), but I have one additional goal that
is harder to satisfy: I'd like to use the same tests that I've written for host
based testing (using the googletest framework) for embedded testing.

## Backup Plan

If a linker section approach turns out to be too difficult (e.g. it requires
modifying the linker script, and that isn't practical for Arduino users),
another approach is to create a linked list in RAM that is populated at startup
time by global variable constructors. For example, the file exposing the
collection might have the following:

```
// In t.h:
class T {
 public:
  T();
  static void DoEverything();

 private:
  virtual void DoOneThing() = 0;

  const T* next_t_;
};

// In t.cc:
namespace {
T* list_of_t;  // Default initialized to nullptr, which happens before
               // constructors are executed.
}

T::T() {
  // Add this instance to the head of the list.
  next_t = list_of_t;
  list_of_t = this;
}

void T::DoEverything() {
  T* ptr = list_of_t;
  while (ptr != nullptr) {
    ptr->DoOneThing();
    ptr = ptr->next_t_;
  }
}

// Some other source file that includes t.h:

class SomeT : public T {
 private:
  void DoOneThing() override { /* do the thing */ }
} SingletonSomeT;

```
