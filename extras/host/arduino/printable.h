#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_PRINTABLE_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_PRINTABLE_H_

#include <stddef.h>

class Print;

class Printable {
 public:
  virtual ~Printable();
  virtual size_t printTo(Print& p) const = 0;
};

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_PRINTABLE_H_
