#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_UDP_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_UDP_H_

// Declares the subset of Arduino's Stream needed for Tiny Alpaca Server,
// especially for host development, where the point is really just to allow for
// compilation and unit testing, not full program testing. Does not redeclare
// virtual methods from the base classes, Stream and Print.

#include <stddef.h>
#include <stdint.h>

#include "extras/host/arduino/ip_address.h"
#include "extras/host/arduino/stream.h"

class UDP : public Stream {
 public:
  virtual uint8_t begin(uint16_t port) = 0;
  virtual uint8_t beginMulticast(IPAddress, uint16_t) {
    return 0;  // Zero means failure, i.e. an impl doesn't need to support it.
  }
  virtual void stop() = 0;

  // Sending...
  virtual int beginPacket(IPAddress ip, uint16_t port) = 0;
  virtual int beginPacket(const char* host, uint16_t port) = 0;
  virtual int endPacket() = 0;

  // Receiving...
  virtual int parsePacket() = 0;
  using Stream::read;
  virtual int read(unsigned char* buffer, size_t len) = 0;
  virtual int read(char* buffer, size_t len) = 0;
  // Note that flush is declared in Print, where it relates to flushing the
  // output, but here it means that we're done reading from the current packet
  // (i.e. any unread portion is to be tossed out so that the next packet can be
  // read).
  using Stream::flush;
};

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_UDP_H_
