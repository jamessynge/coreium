#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_CALL_SETUP_AND_LOOP_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_CALL_SETUP_AND_LOOP_H_

namespace mcucore_host {

// Provides a host version of the Arduino code that calls setup() and loop(),
// with these additional features:
//
//   Uses the flag --num_loops to decide how many times to call loop().
//
//   Uses the flag --loop_sleep to decide how long to sleep after each loop().
//
//   Uses the flag --watchdog_timeout to decide whether setup() or loop() has
//   gotten stuck, and if so it CHECK fails so that a stack dump is printed;
//   maybe use a signal to do so for just the main thread?
void CallSetupAndLoop();

}  // namespace mcucore_host

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_CALL_SETUP_AND_LOOP_H_
