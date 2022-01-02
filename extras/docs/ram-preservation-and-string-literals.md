# String Literals in Arduino Sketches

The Arduino Uno has a mere 2KB of RAM, and an Arduino Mega has only 8KB of RAM.
Unless extra effort is taken, it is easy to consume lots of that RAM by
accident. Of particular concern are string literals (e.g. strings used for
logging messages or for comparison against input strings). By default, string
literals (and other array and struct literals) are automatically copied from
Flash memory into RAM when an Arduino sketch starts running. To avoid this, we
can annotate our literals with PROGMEM (i.e. PROGram MEMory, aka Flash); details
can be read at the
[avr-libc site](https://www.nongnu.org/avr-libc/user-manual/pgmspace.html)).

## Harvard Architecture

*(Memory, not Buildings)*

It's a bit complex to use PROGMEM strings because of the multiple address spaces
used by the AVR chips (e.g. the ATmega 2560 of the Arduino Mega). These chips
have what is called a (modified) Harvard Architecture, which is one in which
there is a not a single address space containing all of the program and data,
but instead the same address (e.g. 0x123) can refer to either a location in RAM
or to a location in PROGMEM, but the address alone doesn't tell you which it is.
Instead the your program needs to keep track of this for every address you're
dealing with.

Furthemore, the string functions of the standard C library (e.g. `strlen`)
provided by avr-libc only work on RAM, not PROGMEM. So if you wanted to compare
a null terminated string in RAM against a null terminated string stored in
flash, `strcmp` wouldn't work for you. To help with this, avr-libc provides a
second set of string functions for situations like this, declared in
`[pgmspace.h](https://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html)`
(e.g. `strcmp_P` for comparing a RAM string against a PROGMEM string).

## Memory Space Awareness via Type Safety

The Arduino libraries provide a minimal form of type safety for PROGMEM strings
by way of the macro `F(str)`, which indicates that the string literal `str` is
to be stored in PROGMEM, and changing its type to `const __FlashStringHelper*`.
The Arduino libraries provide two classes which support this:

*   `Print`: Supports printing a string stored in PROGMEM IFF its type is
    `__FlashStringHelper*`.
*   `String`: Supports copying a string stored in PROGMEM into the char array
    owned by a String instance (i.e. into RAM).

> **Note:** The Arduino libraries have a forward declaration for `class
> __FlashStringHelper`, but not a definition of the class. This is because it is
> just used to provide a distinct type for pointers to strings in PROGMEM, one
> that can't be implicitly converted to `const char*`.

McuCore provides classes `ProgmemStringView`, `ProgmemStringData`, etc., for
representing strings stored in PROGMEM, and class `StringView` for strings
stored in RAM, along with a selection of member functions and free functions for
operating on pairs of such strings. For example:

*   `string_compare.h` provides functions for comparing or searching RAM and
    PROGMEM strings, such as `CaseEqual` for performing a case-insensitive
    comparison between a RAM string and a PROGMEM string.
*   `OPrintStream` is like C++'s std::ostream, providing the ability to stream
    values of various types (numbers, RAM and PROGMEM strings, anything that has
    an interface compatible with Arduino's class `Printable`.
*   `logging.h` provides support for logging messages (e.g. to Arduino's
    `Serial` instance), building on `OPrintStream`.
*   `json_encoder.h` provides support for streaming of JSON encoded objects and
    arrays, where the strings used for values or property names can be in RAM or
    PROGMEM.

## De-Duplicating Strings

Arduino's `F(str)` macro annotates a string literal to keep it from being copied
to RAM, but otherwise it is like any other C or C++ string literal: each literal
gets its own space in the program, even if two (or 10) literals have the same
exact value. An Arduino UNO has only 32KB of Flash, and an Arduino Mega has only
256KB of Flash, so its best to avoid wasting that space unnecessarily.

To address this, `progmem_string_data.h` provides macros that arrange for
multiple copies of a string literal, even it in multiple source files, to share
the same storage in Flash.

*   `MCU_FLASHSTR(str)` can be used as a direct replacement for `F(str)`.
*   `MCU_PSV(str)` expands to a ProgmemStringView instance with the string
    literal `str` as the value it views.
*   `MCU_BASENAME(str)` is like `MCU_FLASHSTR`, but removes from the string any
    slash characters (forward or backward), and any characters that appear
    before them in the string. This means that `/my/directory/file-name.cpp`
    becomes `file-name.cpp`. This macro is used by `logging.h` to enable the
    source file location of a logging statement to be logged along with the
    message.

> **Note:** I came across (and lost track of) an article which showed how to use
> inline assembler in C++ for the AVR chips such that overlapping duplicate
> strings share the same Flash memory, but that was a distinctly non-portable
> solution.

# MORE TEXT TO BE CLEANED UP

## Defining Literals

1.  Defining them at file scope in .cc files using the above macros; for
    example:

    ```
    constexpr auto kFalseFlashStr = MCU_FLASHSTR("false");
    constexpr auto kOKStrView = MCU_PSV("OK");
    ```

1.  Using the `MCU_PSV(value)` macro inline in expressions, such as:

    ```
    MCU_CHECK(false) << MCU_PSV("api group (") << group
                     << MCU_PSV(") is not device or setup");
    ```

    Unlike the above macros, this defines a full specialization of the variadic
    class template `ProgmemStringStorage`. The template declares a static
    function MakeProgmemStringView that returns a mcucore::ProgmemStringView
    instance. The storage class has a static array holding the characters of the
    string, without a terminating NUL. One advantage of MCU_PSV over
    TAS_DEFINE_PROGMEM_LITERAL is that the compiler and linker should collapse
    multiple occurrences of TASLIT(x) with the same value of x. A downside of
    using MCU_PSV is that it uses some fancy compile time type deduction to
    determine the length of the string, and this slows compilation. If you use
    the string in multiple files, defining it in a shared source file and
    exposing them via TAS_DEFINE_PROGMEM_LITERAL will reduce compile time.

1.  Using the `MCU_FLASHSTR(value)` macro inline in expressions, such as:

    ```
    MCU_CHECK(false) << MCU_FLASHSTR("api group (") << group
                     << MCU_FLASHSTR(") is not device or setup");
    ```

    `MCU_FLASHSTR` shares much in common with TASLIT, but the return type is
    `const __FlashStringHelper*`, the type defined by Arduino to denote a string
    stored in flash memory, which (on Harvard architecture processors) can't be
    treated as a regular string. In general, it appears that `MCU_FLASHSTR` is
    the most convenient of these macros.

After building up the facilities described above (not including MCU_FLASHSTR), I
learned that the macro `PSTR(value)` (from AVR libc's pgmspace.h) will
(supposedly) have the effect of storing a string in PROGMEM only. Further, the
developers built on this by defining the macro `F(value)` in WString.h which
performs a typecast on a `PSTR(value)` to give it the type `const
__FlashStringHelper*`.

```
class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))
```

This is good for printing a string, but not for comparing it with a
`mcucore::StringView`. Further, it doesn't capture the string length at compile
time, so it must be rediscovered each time.

When trying to use `F(value)` and `PSTR(value)`, I discovered that they do what
they say, but don't collapse multiple occurrences of a string into one. Hence
`MCU_FLASHSTR(value)` is preferred over `F(value)` if you just want to output a
string, and `TASLIT(value)` is preferred if you also want to know the length
without search the string for the terminating NUL.
