#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_WCHARACTER_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_WCHARACTER_H_

// The subset of Arduino's WCharacter.h that I need.

// Exported because headers in the Arduino core for AVR include it, so it is
// included for everything that includes <Arduino.h>.
#include <ctype.h>  // IWYU pragma: export

// ASCII character classification functions. The AVR implementations are written
// in assembly, should be pretty quick. The one's I've looked at are not table
// based, so using as few as possible minimizes Flash usage.

bool isAlphaNumeric(const char c);  // C isalnum()
bool isGraph(const char c);         // C isgraph()
bool isPrintable(const char c);     // C isprint()
bool isUpperCase(const char c);     // C isupper()

/* The full set documented by Arduino:
isAlpha()
isAlphaNumeric()
isAscii()
isControl()
isDigit()
isGraph()
isHexadecimalDigit()
isLowerCase()
isPrintable()
isPunct()
isSpace()
isUpperCase()
isWhitespace()
*/

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_WCHARACTER_H_
