#ifndef ARDUINO_MCUCORE_EXTRAS_HOST_MLX90614_MLX90614_H_
#define ARDUINO_MCUCORE_EXTRAS_HOST_MLX90614_MLX90614_H_

// Minimal fake version of Adafruit's MLX90614 library.

#include "extras/host/arduino/arduino.h"

class Adafruit_MLX90614 {
 public:
  Adafruit_MLX90614() {}
  bool begin() { return true; }

  double readObjectTempC();
  double readAmbientTempC();
};

#endif  // ARDUINO_MCUCORE_EXTRAS_HOST_MLX90614_MLX90614_H_
