#ifndef MCUCORE_EXTRAS_HOST_SPI_SPI_H_
#define MCUCORE_EXTRAS_HOST_SPI_SPI_H_

// Placeholder classes to allow compiling, but not running, code that uses the
// Arduino SPI library, e.g. ArduinoCore-avr/libraries/SPI/src/SPI.h.

#include <cstddef>
#include <cstdint>

#define SPI_HAS_TRANSACTION 1

class SPISettings {
 public:
  SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {}
  SPISettings() = default;
};

class SPIClass {
 public:
  // We could change these from static to virtual to allow for mocking on host.
  static void begin() {}
  static void usingInterrupt(uint8_t) {}
  static void beginTransaction(SPISettings settings) {}
  static uint8_t transfer(uint8_t data) { return data; }
  static uint16_t transfer16(uint16_t data) { return data; }
  static void transfer(void *buf, size_t count) {}
  static void setTransferWriteFill(uint8_t ch) {}
  static void transfer(const void *buf, void *retbuf, uint32_t count) {}
  static void endTransaction() {}
  static void end() {}
  static void setBitOrder(uint8_t bitOrder) {}
  static void setDataMode(uint8_t dataMode) {}
};

extern SPIClass SPI;

#endif  // MCUCORE_EXTRAS_HOST_SPI_SPI_H_
