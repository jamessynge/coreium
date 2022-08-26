#ifndef MCUCORE_EXTRAS_HOST_MLX90614_MLX90614_H_
#define MCUCORE_EXTRAS_HOST_MLX90614_MLX90614_H_

// Minimal fake version of Adafruit's MLX90614 library.
//
// Author: james.synge@gmail.com

#include "extras/host/arduino/arduino.h"

class Adafruit_MLX90614 {
 public:
  Adafruit_MLX90614() {}
  bool begin() { return true; }

  double readObjectTempC() { return 0.0; }
  double readAmbientTempC() { return 20.0; }
};

#endif  // MCUCORE_EXTRAS_HOST_MLX90614_MLX90614_H_
