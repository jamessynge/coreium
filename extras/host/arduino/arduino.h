#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_ARDUINO_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_ARDUINO_H_

// This file, and the others in this directory, support developing and partially
// testing Arduino libraries and sketches with the host as the target, rather
// than always needing to upload to the embedded target for testing.
//
// This is a partial copy, partial adaptation of Arduino.h from the Arduino core
// for AVR chips; the original file, and its copy right declaration, can be
// found here:
//
//    https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Arduino.h
//
// That header defines lots of macros, types and functions, some of which have
// the same names as functions or symbols in the standard C or C++ library, and
// some of those (especially macros such as abs, min and max) interfere with the
// use of such libraries.
//
// To make sure that I detect problems during host development, rather than
// later when compiling for the embedded target, I've copied various parts of
// the original file here, and modified as follows to achieve my goals:
//
// *  Many functions are defined here as inline with empty or trivial bodies.
// *  Some functions are implemented in arduino.cc, such as millis(), using
//    suitable host libraries (e.g. Abseil's absl::Now()).
// *  The definitions of some macros, such as noInterrupt, are changed so that
//    they're no-ops; they're not converted to inline functions so that name
//    collisions will still be detected.
// *  Integer types used by Arduino (e.g. int or unsigned long) are replaced by
//    type aliases (i.e. ArduinoInt and ArduinoULong, respectively), so that the
//    sizes used on host match those of the Arduino Uno (AVR ATmega328P) and
//    Arduino Mega 2560 (AVR ATmega2560), where an int is 16 bits, and a long
//    is 32 bits.
// *  The set of standard C includes is not quite the same, nor is the order.
//
// Author of the modifications: james.synge@gmail.com
//------------------------------------------------------------------------------

// This file should only be used for compiling Arduino code for a host target,
// e.g. for host based testing of code intended to run on an Arduino embedded
// target.
#ifdef ARDUINO
#error("Why is this file be used to target an Arduino!!")
#endif

// This file includes headers to emulate what happens in the "real" Arduino.h,
// so we have includes that aren't used within this file.
// NOLINTBEGIN(clangd-unused-includes)
// IWYU pragma: begin_exports

#include <math.h>     // IWYU pragma: export
#include <stdbool.h>  // IWYU pragma: export
#include <stdint.h>   // IWYU pragma: export
#include <stdlib.h>   // IWYU pragma: export
#include <string.h>   // IWYU pragma: export

using ArduinoInt = int16_t;
using ArduinoUInt = uint16_t;
using ArduinoLong = int32_t;
using ArduinoULong = uint32_t;
// We 'could' use this in place of size_t, but some work will likely be required
// to do so (i.e. it isn't just search and replace).
using ArduinoSizeT = ArduinoUInt;

// CPU specific headers define the clock ticks/second; I've specified a value
// here that is common for many Arduino CPUs (i.e. for Arduino UNO and Mega).
#define F_CPU 16000000

// These AVR specific libraries are included directly or indirectly by
// Arduino.h.

#include "extras/host/arduino/avr_io.h"    // IWYU pragma: export
#include "extras/host/arduino/pgmspace.h"  // IWYU pragma: export

// #include avr/interrupt  // Not needed on host.

// These Arduino specific libraries are included directly or indirectly by
// Arduino.h.

// #include "Binary.h"  // Not needed in my code.

// #ifdef __cplusplus
// extern "C" {
// #endif

inline void yield() {}

#define HIGH 0x1
#define LOW 0x0

// I don't use just 0, 1 and 2 here so that I detect issues with assuming
// specific values. Heck, I could even have these replaced by calls to return
// a different value on each program execution.
#define INPUT 97
#define INPUT_PULLUP 98
#define OUTPUT 99

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER 2.718281828459045235360287471352

#define SERIAL 0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

// I don't use just 1, 2 and 3 here so that I detect issues with assuming
// specific values.
#define CHANGE 7
#define FALLING 8
#define RISING 9

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || \
    defined(__AVR_ATtiny84__)
#define DEFAULT 0
#define EXTERNAL 1
#define INTERNAL1V1 2
#define INTERNAL INTERNAL1V1
#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || \
    defined(__AVR_ATtiny85__)
#define DEFAULT 0
#define EXTERNAL 4
#define INTERNAL1V1 8
#define INTERNAL INTERNAL1V1
#define INTERNAL2V56 9
#define INTERNAL2V56_EXTCAP 13
#else
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) ||  \
    defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || \
    defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) ||   \
    defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
#define INTERNAL1V1 2
#define INTERNAL2V56 3
#else
#define INTERNAL 3
#endif
#define DEFAULT 1
#define EXTERNAL 0
#endif

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

// NOTE: I am NOT defining min, max, abs, and round here, and possibly some
// other macros, because they get in the way of compiling host-only code, such
// as that which includes <ostream>, because that indirectly includes <chrono>,
// which defines a function called 'round'.

// #define min(a, b) ((a) < (b) ? (a) : (b))
// #define max(a, b) ((a) > (b) ? (a) : (b))
// #define abs(x) ((x) > 0 ? (x) : -(x))
#define constrain(amt, low, high) \
  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
// #define round(x) ((x) >= 0 ? (ArduinoLong)((x) + 0.5) :
// (ArduinoLong)((x)-0.5))
#define radians(deg) ((deg) * DEG_TO_RAD)
#define degrees(rad) ((rad) * RAD_TO_DEG)
#define sq(x) ((x) * (x))

// These are no-ops, expressed as void expressions.
#define interrupts() (void)0
#define noInterrupts() (void)0

#define clockCyclesPerMicrosecond() (F_CPU / 1000000L)
#define clockCyclesToMicroseconds(a) ((a) / clockCyclesPerMicrosecond())
#define microsecondsToClockCycles(a) ((a) * clockCyclesPerMicrosecond())

#define lowByte(w) ((uint8_t)((w) & 0xff))
#define highByte(w) ((uint8_t)((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) \
  ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

// avr-libc defines _NOP() since 1.6.2
#ifndef _NOP
#define _NOP()               \
  do {                       \
    __asm__ volatile("nop"); \
  } while (0)
#endif

using word = ArduinoUInt;

#define bit(b) (1UL << (b))

typedef bool boolean;
typedef uint8_t byte;

inline void init() {}
inline void initVariant() {}

// atexit is defined by stdlib.h.
// ArduinoInt atexit(void (*func)()) __attribute__((weak));

inline void pinMode(uint8_t pin, uint8_t mode) {}
inline void digitalWrite(uint8_t pin, uint8_t val) {}
inline ArduinoInt digitalRead(uint8_t pin) { return HIGH; }
inline ArduinoInt analogRead(uint8_t pin) { return 123; }
inline void analogReference(uint8_t mode) {}
inline void analogWrite(uint8_t pin, ArduinoInt val) {}

ArduinoULong millis();
ArduinoULong micros();
void delay(ArduinoULong ms);
void delayMicroseconds(ArduinoUInt us);
ArduinoULong pulseIn(uint8_t pin, uint8_t state, ArduinoULong timeout);
ArduinoULong pulseInLong(uint8_t pin, uint8_t state, ArduinoULong timeout);

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

inline void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void),
                            ArduinoInt mode) {}
inline void detachInterrupt(uint8_t interruptNum) {}

// void setup();
// void loop();

// Returning a variety of values, but totally bogus.
#define digitalPinToInterrupt(p) \
  ((p) == 2                      \
       ? 0                       \
       : ((p) == 3 ? 1           \
                   : ((p) >= 18 && (p) <= 21 ? 23 - (p) : NOT_AN_INTERRUPT)))

// Get the bit location within the hardware port of the given virtual pin.
#define digitalPinToPort(p) (p)
#define analogInPinToBit(p) (p)

#define digitalPinToBitMask(p) 1

#define digitalPinToTimer(p) 1

#define portOutputRegister(p) &TCCR1A  // Totally BOGUS
#define portInputRegister(p) &TCCR1A   // Totally BOGUS
#define portModeRegister(p) &TCCR1A    // Totally BOGUS

#define ISR(name) void ISR_##name()

// From avr-libc's include/avr/sfr_defs.h
#define _BV(bit) (1 << (bit))

#define NOT_A_PIN 0
#define NOT_A_PORT 0

#define NOT_AN_INTERRUPT -1

#ifdef ARDUINO_MAIN
#define PA 1
#define PB 2
#define PC 3
#define PD 4
#define PE 5
#define PF 6
#define PG 7
#define PH 8
#define PJ 10
#define PK 11
#define PL 12
#endif  // ARDUINO_MAIN

#define NOT_ON_TIMER 0
#define TIMER0A 1
#define TIMER0B 2
#define TIMER1A 3
#define TIMER1B 4
#define TIMER1C 5
#define TIMER2 6
#define TIMER2A 7
#define TIMER2B 8

#define TIMER3A 9
#define TIMER3B 10
#define TIMER3C 11
#define TIMER4A 12
#define TIMER4B 13
#define TIMER4C 14
#define TIMER4D 15
#define TIMER5A 16
#define TIMER5B 17
#define TIMER5C 18

// #ifdef __cplusplus
// }  // extern "C"
// #endif
// #ifdef __cplusplus

// #include "WCharacter.h" provides wrappers around <ctype.h>, so I should use
// those directly. On the other hand, the ctype impl in avr-libc is a bunch of
// assembly language code rather than a lookup table. IFF short on flash,
// consider whether it would be smaller if I wrote my own for the few functions
// I need while decoding a request.
#include "extras/host/arduino/wcharacter.h"  // IWYU pragma: export
#include "extras/host/arduino/wstring.h"     // IWYU pragma: export

// Arduino's HardwareSerial.h includes Stream.h, which in turn includes Print.h.
// I'm explicitly including them here to make it easier to work with IWYU.
#include "extras/host/arduino/print.h"   // IWYU pragma: export
#include "extras/host/arduino/random.h"  // IWYU pragma: export
#include "extras/host/arduino/serial.h"  // IWYU pragma: export
#include "extras/host/arduino/stream.h"  // IWYU pragma: export

// #include "WCharacter.h"
// #include "WString.h"
// #include "HardwareSerial.h"
// #include "USBAPI.h"

#if defined(HAVE_HWSERIAL0) && defined(HAVE_CDCSERIAL)
#error "Targets with both UART0 and CDC serial not supported"
#endif

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)

ArduinoULong pulseIn(uint8_t pin, uint8_t state,
                     ArduinoULong timeout = 1000000L);
ArduinoULong pulseInLong(uint8_t pin, uint8_t state,
                         ArduinoULong timeout = 1000000L);

void tone(uint8_t _pin, ArduinoUInt frequency, ArduinoULong duration = 0);
void noTone(uint8_t _pin);

// WMath prototypes
ArduinoLong random(ArduinoLong);
ArduinoLong random(ArduinoLong, ArduinoLong);
void randomSeed(ArduinoULong);
ArduinoLong map(ArduinoLong, ArduinoLong, ArduinoLong, ArduinoLong,
                ArduinoLong);

// #endif  // __cplusplus

// IWYU pragma: end_exports
// NOLINTEND(clangd-unused-includes)

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_ARDUINO_H_
