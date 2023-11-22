#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_SERVER_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_SERVER_H_

// Arduino defines a rather odd Server class that is just Print plus a begin
// method. It doesn't appear to be documented, but it is referenced by w5500.
// Sigh.

#include "extras/host/arduino/print.h"

class Server : public Print {
 public:
  virtual void begin() = 0;
};

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_SERVER_H_
