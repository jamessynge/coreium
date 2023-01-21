load(
    "//mcucore/extras/bazel:arduino_cc_sketch.bzl",
    "arduino_cc_sketch",
)

arduino_cc_sketch(
    name = "study_watchdog",
    srcs = ["study_watchdog.ino.cc"],
    deps = [
        "//mcucore/extras/host/arduino:arduino_main",
        "//mcucore/src:McuCore",
    ],
)
