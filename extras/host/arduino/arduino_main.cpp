// Provides the minimal wrapper around setup() and loop()

#include "base/init_google.h"
#include "extras/host/arduino/call_setup_and_loop.h"

int main(int argc, char* argv[]) {
  InitGoogle(argv[0], &argc, &argv, /*remove_flags=*/true);
  mcucore_host::CallSetupAndLoop();
  return 0;
}
