#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_STREAM_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_STREAM_H_

// Declares the subset of Arduino's Stream needed for Tiny Alpaca Server.
// Many other non-virtual methods are omitted that are in Arduino's Stream.
// Fortunately none have the same name as one of the virtual methods, so it
// isn't necessary to declare them, nor to add using declarations in derived
// classes which implement these virtual functions.

#include "extras/host/arduino/print.h"

class Stream : public Print {
 public:
  // Returns the number of bytes available to read currently.
  virtual int available() = 0;

  // Reads (consumes from the stream) and returns the first byte of incoming
  // data available, or -1 if no data is available currently (i.e. this is a
  // non-blocking method).
  virtual int read() = 0;

  // Returns, but does not consume, the first byte of incoming data available,
  // or -1 if no data is available currently (i.e. this is a
  // non-blocking method).
  virtual int peek() = 0;
};

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_STREAM_H_
