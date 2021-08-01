#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_CHARACTER_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_CHARACTER_H_

// The subset of Arduino's WCharacter.h that I need.

bool isPrintable(const char c);
bool isAlphaNumeric(const char c);
bool isUpperCase(const char c);

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_CHARACTER_H_
