#include "extras/host/arduino/call_setup_and_loop.h"

#include <unistd.h>

#include <csignal>

#include "absl/flags/flag.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/time/clock.h"

ABSL_FLAG(int, num_loops, 1, "Number of times to execute loop");
ABSL_FLAG(absl::Duration, loop_sleep, absl::Milliseconds(300),
          "Time to sleep at the end of each loop.");
ABSL_FLAG(int, watchdog_timeout_secs, 5,
          "Runtime limit (seconds) for setup and for each loop.");

void setup();  // To be provided by the "sketch" that links in this library.
void loop();   // To be provided by the "sketch" that links in this library.

namespace mcucore_host {
namespace {

void HandleAlarm(int sig) { CHECK(false) << "SIGALRM caught!"; }

template <typename Function>
void RunWithAlarm(const int seconds, Function f) {
  struct sigaction saved_action;
  if (seconds > 0) {
    // Arrange for HandleAlarm to be called if f() runs for longer than seconds.
    struct sigaction action;
    action.sa_handler = HandleAlarm;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGALRM, &action, &saved_action);
    alarm(seconds);
  }

  f();

  if (seconds > 0) {
    // Disable the alarm.
    alarm(0);
    sigaction(SIGALRM, &saved_action, nullptr);
  }
}
}  // namespace

void CallSetupAndLoop() {
  int num_loops = absl::GetFlag(FLAGS_num_loops);
  const absl::Duration loop_sleep = absl::GetFlag(FLAGS_loop_sleep);
  const int watchdog_timeout_secs = absl::GetFlag(FLAGS_watchdog_timeout_secs);

  LOG(INFO) << "Number of loops: " << num_loops;
  LOG(INFO) << "Loop sleep: " << loop_sleep;
  LOG(INFO) << "SIGALRM seconds: " << watchdog_timeout_secs;

  RunWithAlarm(watchdog_timeout_secs, setup);
  while (true) {
    RunWithAlarm(watchdog_timeout_secs, loop);
    if (num_loops > 0) {
      num_loops--;
    }
    if (num_loops == 0) {
      break;
    }
    if (loop_sleep > absl::ZeroDuration()) {
      absl::SleepFor(loop_sleep);
    }
  }
}

}  // namespace mcucore_host