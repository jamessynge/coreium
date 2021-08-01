#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_RANDOM_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_RANDOM_H_

#include <stdint.h>

// Generate pseudo-random numbers.

// Returns a random number in the range [0, howbig-1].
int32_t random(int32_t howbig);

// Returns a random number in the range [howsmall, howbig-1].
int32_t random(int32_t howsmall, int32_t howbig);

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_RANDOM_H_
